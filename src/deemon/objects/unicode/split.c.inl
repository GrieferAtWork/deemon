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
#ifndef GUARD_DEEMON_OBJECTS_UNICODE_SPLIT_C_INL
#define GUARD_DEEMON_OBJECTS_UNICODE_SPLIT_C_INL 1

#ifdef __INTELLISENSE__
#include "string_functions.c"
#endif

#include <deemon/seq.h>

DECL_BEGIN

typedef struct {
    OBJECT_HEAD
    DREF DeeStringObject *s_str; /* [1..1][const] The string that is being split. */
    DREF DeeStringObject *s_sep; /* [1..1][const] The string to search for. */
} StringSplit;

typedef struct {
    OBJECT_HEAD
    DREF StringSplit *s_split; /* [1..1][const] The split descriptor object. */
    uint8_t          *s_next;  /* [0..1][atomic] Pointer to the starting address of the next split
                                *               (points into the s_enc-specific string of `s_split->s_str')
                                *                When the iterator is exhausted, this pointer is set to `NULL'. */
    uint8_t          *s_begin; /* [1..1][const] The starting address of the width string of `s_split->s_str'. */
    uint8_t          *s_end;   /* [1..1][const] The end address of the width string of `s_split->s_str'. */
    uint8_t          *s_sep;   /* [1..1][const] The starting address of the `s_enc'-encoded string of `s_split->s_sep'. */
    size_t            s_sepsz; /* [1..1][const][== WSTR_LENGTH(s_sep)] The length of seperator string. */
    int               s_width; /* [const] The width of `s_split->s_str' */
} StringSplitIterator;

PRIVATE DREF DeeObject *DCALL
splititer_next(StringSplitIterator *__restrict self) {
 uint8_t *result_start,*result_end;
 uint8_t *next_ptr; size_t result_len;
 do {
  result_start = ATOMIC_READ(self->s_next);
  if (!result_start) return ITER_DONE;
  SWITCH_SIZEOF_WIDTH(self->s_width) {
  CASE_WIDTH_1BYTE:
   result_end = (uint8_t *)memmemb((uint8_t *)result_start,
                                   (uint8_t *)self->s_end-(uint8_t *)result_start,
                                   (uint8_t *)self->s_sep,self->s_sepsz);
   if (!result_end)
        result_end = self->s_end,
        next_ptr = NULL;
   else next_ptr = result_end+self->s_sepsz*1;
   result_len = (size_t)((uint8_t *)result_end-(uint8_t *)result_start);
   break;
  CASE_WIDTH_2BYTE:
   result_end = (uint8_t *)memmemw((uint16_t *)result_start,
                                   (uint16_t *)self->s_end-(uint16_t *)result_start,
                                   (uint16_t *)self->s_sep,self->s_sepsz);
   if (!result_end)
        result_end = self->s_end,
        next_ptr = NULL;
   else next_ptr = result_end+self->s_sepsz*2;
   result_len = (size_t)((uint16_t *)result_end-(uint16_t *)result_start);
   break;
  CASE_WIDTH_4BYTE:
   result_end = (uint8_t *)memmeml((uint32_t *)result_start,
                                   (uint32_t *)self->s_end-(uint32_t *)result_start,
                                   (uint32_t *)self->s_sep,self->s_sepsz);
   if (!result_end)
        result_end = self->s_end,
        next_ptr = NULL;
   else next_ptr = result_end+self->s_sepsz*4;
   result_len = (size_t)((uint32_t *)result_end-(uint32_t *)result_start);
   break;
  }
 } while (!ATOMIC_CMPXCH(self->s_next,result_start,next_ptr));
 /* Return the part-string. */
 return DeeString_NewWithWidth(result_start,
                               result_len,
                               self->s_width);
}

PRIVATE DREF DeeObject *DCALL
casesplititer_next(StringSplitIterator *__restrict self) {
 /* Literally the same as the non-case version, but use `memcasemem(b|w|l)' instead. */
 uint8_t *result_start,*result_end;
 uint8_t *next_ptr; size_t result_len,match_length;
 do {
  result_start = ATOMIC_READ(self->s_next);
  if (!result_start) return ITER_DONE;
  SWITCH_SIZEOF_WIDTH(self->s_width) {
  CASE_WIDTH_1BYTE:
   result_end = (uint8_t *)memcasememb((uint8_t *)result_start,
                                       (uint8_t *)self->s_end-(uint8_t *)result_start,
                                       (uint8_t *)self->s_sep,self->s_sepsz,
                                       &match_length);
   if (!result_end)
        result_end = self->s_end,
        next_ptr = NULL;
   else next_ptr = result_end+match_length*1;
   result_len = (size_t)((uint8_t *)result_end-(uint8_t *)result_start);
   break;
  CASE_WIDTH_2BYTE:
   result_end = (uint8_t *)memcasememw((uint16_t *)result_start,
                                       (uint16_t *)self->s_end-(uint16_t *)result_start,
                                       (uint16_t *)self->s_sep,self->s_sepsz,
                                       &match_length);
   if (!result_end)
        result_end = self->s_end,
        next_ptr = NULL;
   else next_ptr = result_end+match_length*2;
   result_len = (size_t)((uint16_t *)result_end-(uint16_t *)result_start);
   break;
  CASE_WIDTH_4BYTE:
   result_end = (uint8_t *)memcasememl((uint32_t *)result_start,
                                       (uint32_t *)self->s_end-(uint32_t *)result_start,
                                       (uint32_t *)self->s_sep,self->s_sepsz,
                                       &match_length);
   if (!result_end)
        result_end = self->s_end,
        next_ptr = NULL;
   else next_ptr = result_end+match_length*4;
   result_len = (size_t)((uint32_t *)result_end-(uint32_t *)result_start);
   break;
  }
 } while (!ATOMIC_CMPXCH(self->s_next,result_start,next_ptr));
 /* Return the part-string. */
 return DeeString_NewWithWidth(result_start,
                               result_len,
                               self->s_width);
}


PRIVATE void DCALL
splititer_fini(StringSplitIterator *__restrict self) {
 Dee_Decref(self->s_split);
}
#ifdef CONFIG_NO_THREADS
#define GET_SPLIT_NEXT(x)            ((x)->s_next)
#else
#define GET_SPLIT_NEXT(x) ATOMIC_READ((x)->s_next)
#endif

PRIVATE int DCALL
splititer_bool(StringSplitIterator *__restrict self) {
 return GET_SPLIT_NEXT(self) != NULL;
}

#define DEFINE_SPLITITER_CMP(name,op) \
PRIVATE DREF DeeObject * \
name(StringSplitIterator *__restrict self, \
     StringSplitIterator *__restrict other) { \
 if (DeeObject_AssertTypeExact((DeeObject *)other,Dee_TYPE(self))) \
     return NULL; \
 return_bool(GET_SPLIT_NEXT(self) op GET_SPLIT_NEXT(other)); \
}
DEFINE_SPLITITER_CMP(splititer_eq,==)
DEFINE_SPLITITER_CMP(splititer_ne,!=)
DEFINE_SPLITITER_CMP(splititer_lo,<)
DEFINE_SPLITITER_CMP(splititer_le,<=)
DEFINE_SPLITITER_CMP(splititer_gr,>)
DEFINE_SPLITITER_CMP(splititer_ge,>=)
#undef DEFINE_SPLITITER_CMP

PRIVATE struct type_cmp splititer_cmp = {
     /* .tp_hash = */(dhash_t(DCALL *)(DeeObject *__restrict))NULL,
     /* .tp_eq   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&splititer_eq,
     /* .tp_ne   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&splititer_ne,
     /* .tp_lo   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&splititer_lo,
     /* .tp_le   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&splititer_le,
     /* .tp_gr   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&splititer_gr,
     /* .tp_ge   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&splititer_ge
};


PRIVATE struct type_member splititer_members[] = {
    TYPE_MEMBER_FIELD("seq",STRUCT_OBJECT,offsetof(StringSplitIterator,s_split)),
    TYPE_MEMBER_END
};

PRIVATE int DCALL
splititer_copy(StringSplitIterator *__restrict self,
               StringSplitIterator *__restrict other) {
 self->s_split = other->s_split;
 self->s_next  = GET_SPLIT_NEXT(other);
 self->s_begin = other->s_begin;
 self->s_end   = other->s_end;
 self->s_sep   = other->s_sep;
 self->s_sepsz = other->s_sepsz;
 self->s_width   = other->s_width;
 Dee_Incref(self->s_split);
 return 0;
}

INTERN DeeTypeObject DeeSplitIterator_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"_splititerator",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FNORMAL,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeIterator_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */NULL,
                /* .tp_copy_ctor = */(int(DCALL *)(DeeTypeObject *__restrict,DeeObject *__restrict,DeeObject *__restrict))&splititer_copy,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */NULL,
                /* .tp_free      = */NULL,
                {
                    /* .tp_instance_size = */sizeof(StringSplitIterator)
                }
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&splititer_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */NULL,
        /* .tp_bool = */(int(DCALL *)(DeeObject *__restrict))&splititer_bool
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */NULL, /*  No visit, because it only ever references strings
                                    * (or rather an object that can only reference strings). */
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */&splititer_cmp,
    /* .tp_seq           = */NULL,
    /* .tp_iter_next     = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&splititer_next,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */NULL,
    /* .tp_members       = */splititer_members,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */NULL
};
INTERN DeeTypeObject DeeCaseSplitIterator_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"_casesplititerator",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FNORMAL,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeSplitIterator_Type, /* Extend the regular split() iterator type. */
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */NULL,
                /* .tp_copy_ctor = */NULL,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */NULL,
                /* .tp_free      = */NULL,
                {
                    /* .tp_instance_size = */sizeof(StringSplitIterator)
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
    /* .tp_visit         = */NULL, /*  No visit, because it only ever references strings
                                    * (or rather an object that can only reference strings). */
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */NULL,
    /* .tp_seq           = */NULL,
    /* .tp_iter_next     = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&casesplititer_next,
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



PRIVATE void DCALL
split_fini(StringSplit *__restrict self) {
 Dee_Decref(self->s_str);
 Dee_Decref(self->s_sep);
}

PRIVATE int DCALL
split_bool(StringSplit *__restrict self) {
 return !DeeString_IsEmpty(self->s_str);
}

LOCAL DREF StringSplitIterator *DCALL
split_doiter(StringSplit *__restrict self,
             DeeTypeObject *__restrict iter_type) {
 DREF StringSplitIterator *result;
 result = DeeObject_MALLOC(StringSplitIterator);
 if unlikely(!result) goto done;
 result->s_width = DeeString_WIDTH(self->s_str);
 result->s_begin = (uint8_t *)DeeString_WSTR(self->s_str);
 result->s_next  = result->s_begin;
 result->s_end   = (uint8_t *)DeeString_WEND(self->s_str);
 if (result->s_next == result->s_end)
     result->s_next = NULL;
 /* Load the seperator with the same width as the base string. */
 /* TODO: Use common width */
 result->s_sep   = (uint8_t *)DeeString_AsWidth((DeeObject *)self->s_sep,result->s_width);
 if unlikely(!result->s_sep) goto err_r;
 result->s_sepsz = WSTR_LENGTH(result->s_sep);
 /* Finalize the split iterator and return it. */
 Dee_Incref(self);
 result->s_split = self;
 DeeObject_Init(result,iter_type);
done:
 return result;
err_r:
 DeeObject_Free(result);
 return NULL;
}

PRIVATE DREF StringSplitIterator *DCALL
split_iter(StringSplit *__restrict self) {
 return split_doiter(self,&DeeSplitIterator_Type);
}
PRIVATE DREF StringSplitIterator *DCALL
casesplit_iter(StringSplit *__restrict self) {
 return split_doiter(self,&DeeCaseSplitIterator_Type);
}

PRIVATE struct type_member split_members[] = {
    TYPE_MEMBER_FIELD("__str__",STRUCT_OBJECT,offsetof(StringSplit,s_str)),
    TYPE_MEMBER_FIELD("__sep__",STRUCT_OBJECT,offsetof(StringSplit,s_sep)),
    TYPE_MEMBER_END
};

PRIVATE struct type_member split_class_members[] = {
    TYPE_MEMBER_CONST("iterator",&DeeSplitIterator_Type),
    TYPE_MEMBER_END
};

PRIVATE struct type_member casesplit_class_members[] = {
    TYPE_MEMBER_CONST("iterator",&DeeCaseSplitIterator_Type),
    TYPE_MEMBER_END
};

PRIVATE struct type_seq split_seq = {
    /* .tp_iter_self = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&split_iter,
    /* .tp_size      = */NULL,
    /* .tp_contains  = */NULL,
    /* .tp_get       = */NULL,
    /* .tp_del       = */NULL,
    /* .tp_set       = */NULL,
    /* .tp_range_get = */NULL,
    /* .tp_range_del = */NULL,
    /* .tp_range_set = */NULL
};

PRIVATE struct type_seq casesplit_seq = {
    /* .tp_iter_self = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&casesplit_iter,
    /* .tp_size      = */NULL,
    /* .tp_contains  = */NULL,
    /* .tp_get       = */NULL,
    /* .tp_del       = */NULL,
    /* .tp_set       = */NULL,
    /* .tp_range_get = */NULL,
    /* .tp_range_del = */NULL,
    /* .tp_range_set = */NULL
};

INTERN DeeTypeObject DeeSplit_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"_split",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FNORMAL,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeSeq_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */NULL,
                /* .tp_copy_ctor = */NULL,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */NULL,
                /* .tp_free      = */NULL,
                {
                    /* .tp_instance_size = */sizeof(StringSplit)
                }
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&split_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */NULL,
        /* .tp_bool = */(int(DCALL *)(DeeObject *__restrict))&split_bool
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */NULL, /* No visit, because it only ever references strings. */
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */NULL,
    /* .tp_seq           = */&split_seq,
    /* .tp_iter_next     = */NULL,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */NULL,
    /* .tp_members       = */split_members,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */split_class_members
};

INTERN DeeTypeObject DeeCaseSplit_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"_casesplit",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FNORMAL,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeSplit_Type, /* Extend the regular split() type. */
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */NULL,
                /* .tp_copy_ctor = */NULL,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */NULL,
                /* .tp_free      = */NULL,
                {
                    /* .tp_instance_size = */sizeof(StringSplit)
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
        /* .tp_bool = */(int(DCALL *)(DeeObject *__restrict))&split_bool
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */NULL, /* No visit, because it only ever references strings. */
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */NULL,
    /* .tp_seq           = */&casesplit_seq,
    /* .tp_iter_next     = */NULL,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */NULL,
    /* .tp_members       = */NULL,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */casesplit_class_members
};

/* @return: An abstract sequence type for enumerating the segments of a split string. */
INTERN DREF DeeObject *DCALL
DeeString_Split(DeeObject *__restrict self,
                DeeObject *__restrict seperator) {
 DREF StringSplit *result;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 ASSERT_OBJECT_TYPE_EXACT(seperator,&DeeString_Type);
 result = DeeObject_MALLOC(StringSplit);
 if unlikely(!result) goto done;
 DeeObject_Init(result,&DeeSplit_Type);
 Dee_Incref(self);
 Dee_Incref(seperator);
 result->s_str = (DREF DeeStringObject *)self;      /* Inherit */
 result->s_sep = (DREF DeeStringObject *)seperator; /* Inherit */
done:
 return (DREF DeeObject *)result;
}
/* @return: An abstract sequence type for enumerating the segments of a split string. */
INTERN DREF DeeObject *DCALL
DeeString_CaseSplit(DeeObject *__restrict self,
                    DeeObject *__restrict seperator) {
 DREF StringSplit *result;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 ASSERT_OBJECT_TYPE_EXACT(seperator,&DeeString_Type);
 result = DeeObject_MALLOC(StringSplit);
 if unlikely(!result) goto done;
 /* Same as the regular split(), but use the case-insensitive sequence type. */
 DeeObject_Init(result,&DeeCaseSplit_Type);
 Dee_Incref(self);
 Dee_Incref(seperator);
 result->s_str = (DREF DeeStringObject *)self;      /* Inherit */
 result->s_sep = (DREF DeeStringObject *)seperator; /* Inherit */
done:
 return (DREF DeeObject *)result;
}



typedef struct {
    OBJECT_HEAD
    DREF DeeStringObject *ls_str;   /* [1..1][const] The string that is being split into lines. */
    bool                  ls_keep;  /* [const] True if line-ends should be kept in resulting strings. */
} LineSplit;

typedef struct {
    OBJECT_HEAD
    DREF LineSplit *ls_split; /* [1..1][const] The split descriptor object. */
    uint8_t        *ls_next;  /* [0..1][atomic] Pointer to the starting address of the next split
                               *                (points into the s_enc-specific string of `ls_split->ls_str')
                               *                 When the iterator is exhausted, this pointer is set to NULL. */
    uint8_t        *ls_begin; /* [1..1][const] The starting address of the width string of `ls_split->ls_str'. */
    uint8_t        *ls_end;   /* [1..1][const] The end address of the width string of `ls_split->ls_str'. */
    int             ls_width; /* [const] The width of `ls_split->ls_str' */
    bool            ls_keep;  /* [const] True if line-ends should be kept in resulting strings. */
} LineSplitIterator;

LOCAL uint8_t *DCALL
find_lfb(uint8_t *__restrict start, size_t size) {
 for (;;--size,++start) {
  if (!size) return NULL;
  if (DeeUni_IsLF(*start)) break;
 }
 return start;
}
LOCAL uint16_t *DCALL
find_lfw(uint16_t *__restrict start, size_t size) {
 for (;;--size,++start) {
  if (!size) return NULL;
  if (DeeUni_IsLF(*start)) break;
 }
 return start;
}
LOCAL uint32_t *DCALL
find_lfl(uint32_t *__restrict start, size_t size) {
 for (;;--size,++start) {
  if (!size) return NULL;
  if (DeeUni_IsLF(*start)) break;
 }
 return start;
}


PRIVATE DREF DeeObject *DCALL
lineiter_next(LineSplitIterator *__restrict self) {
 uint8_t *result_start,*result_end;
 uint8_t *next_ptr; size_t result_len;
 do {
  result_start = ATOMIC_READ(self->ls_next);
  if (!result_start) return ITER_DONE;
  SWITCH_SIZEOF_WIDTH(self->ls_width) {
  CASE_WIDTH_1BYTE:
   result_end = (uint8_t *)find_lfb((uint8_t *)result_start,
                                    (uint8_t *)self->ls_end-(uint8_t *)result_start);
   if (!result_end)
        result_end = self->ls_end,
        next_ptr = NULL;
   else {
    next_ptr = result_end+1;
    if (*(uint8_t *)result_end == UNICODE_CR &&
        *(uint8_t *)next_ptr == UNICODE_LF)
          next_ptr += 1;
   }
   result_len = (size_t)((uint8_t *)result_end-
                         (uint8_t *)result_start);
   break;
  CASE_WIDTH_2BYTE:
   result_end = (uint8_t *)find_lfw((uint16_t *)result_start,
                                    (uint16_t *)self->ls_end-(uint16_t *)result_start);
   if (!result_end)
        result_end = self->ls_end,
        next_ptr = NULL;
   else {
    next_ptr = result_end+2;
    if (*(uint16_t *)result_end == UNICODE_CR &&
        *(uint16_t *)next_ptr == UNICODE_LF)
          next_ptr += 2;
   }
   result_len = (size_t)((uint16_t *)result_end-
                         (uint16_t *)result_start);
   break;
  CASE_WIDTH_4BYTE:
   result_end = (uint8_t *)find_lfl((uint32_t *)result_start,
                                    (uint32_t *)self->ls_end-(uint32_t *)result_start);
   if (!result_end)
        result_end = self->ls_end,
        next_ptr = NULL;
   else {
    next_ptr = result_end+4;
    if (*(uint32_t *)result_end == UNICODE_CR &&
        *(uint32_t *)next_ptr == UNICODE_LF)
          next_ptr += 4;
   }
   result_len = (size_t)((uint32_t *)result_end-
                         (uint32_t *)result_start);
   break;
  }
 } while (!ATOMIC_CMPXCH(self->ls_next,result_start,next_ptr));
 /* Add the linefeed itself if we're supposed to include it. */
 if (self->ls_keep && next_ptr)
     result_len += (size_t)(next_ptr-result_end)/STRING_SIZEOF_WIDTH(self->ls_width);
 /* Return the part-string. */
 return DeeString_NewWithWidth(result_start,
                               result_len,
                               self->ls_width);
}


/* Assert that we're allowed to re-use some helper functions from `strsplit' */
STATIC_ASSERT(COMPILER_OFFSETOF(StringSplitIterator,s_split) ==
              COMPILER_OFFSETOF(LineSplitIterator,ls_split));
STATIC_ASSERT(COMPILER_OFFSETOF(StringSplitIterator,s_next) ==
              COMPILER_OFFSETOF(LineSplitIterator,ls_next));


PRIVATE int DCALL
lineiter_copy(LineSplitIterator *__restrict self,
              LineSplitIterator *__restrict other) {
 self->ls_split = other->ls_split;
#ifdef CONFIG_NO_THREADS
 self->ls_next  = other->ls_next;
#else
 self->ls_next  = ATOMIC_READ(other->ls_next);
#endif
 self->ls_begin = other->ls_begin;
 self->ls_end   = other->ls_end;
 self->ls_width   = other->ls_width;
 self->ls_keep  = other->ls_keep;
 Dee_Incref(self->ls_split);
 return 0;
}


INTERN DeeTypeObject DeeLineSplitIterator_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"_linesplititerator",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FNORMAL|TP_FFINAL,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeIterator_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */NULL,
                /* .tp_copy_ctor = */(int(DCALL *)(DeeTypeObject *__restrict,DeeObject *__restrict,DeeObject *__restrict))&lineiter_copy,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */NULL,
                /* .tp_free      = */NULL,
                {
                    /* .tp_instance_size = */sizeof(LineSplitIterator)
                }
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&splititer_fini, /* offset:`s_split' == offset:`ls_split' */
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */NULL,
        /* .tp_bool = */(int(DCALL *)(DeeObject *__restrict))&splititer_bool /* offset:`s_next' == offset:`ls_next' */
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */NULL, /*  No visit, because it only ever references strings
                                    * (or rather an object that can only reference strings). */
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */&splititer_cmp, /* offset:`s_next' == offset:`ls_next' */
    /* .tp_seq           = */NULL,
    /* .tp_iter_next     = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&lineiter_next,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */NULL,
    /* .tp_members       = */splititer_members, /* offset:`s_split' == offset:`ls_split' */
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */NULL
};

PRIVATE struct type_member linesplit_class_members[] = {
    TYPE_MEMBER_CONST("iterator",&DeeLineSplitIterator_Type),
    TYPE_MEMBER_END
};

PRIVATE DREF LineSplitIterator *DCALL
linesplit_iter(LineSplit *__restrict self) {
 DREF LineSplitIterator *result;
 result = DeeObject_MALLOC(LineSplitIterator);
 if unlikely(!result) goto done;
 result->ls_width = DeeString_WIDTH(self->ls_str);
 result->ls_begin = (uint8_t *)DeeString_WSTR(self->ls_str);
 result->ls_next  = result->ls_begin;
 result->ls_end   = (uint8_t *)DeeString_WEND(self->ls_str);
 if (result->ls_next == result->ls_end)
     result->ls_next = NULL;
 result->ls_keep  = self->ls_keep;
 Dee_Incref(self);
 result->ls_split = self;
 DeeObject_Init(result,&DeeLineSplitIterator_Type);
done:
 return result;
}

PRIVATE struct type_seq linesplit_seq = {
    /* .tp_iter_self = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&linesplit_iter
};

PRIVATE void DCALL
linesplit_fini(LineSplit *__restrict self) {
 Dee_Decref(self->ls_str);
}

STATIC_ASSERT(COMPILER_OFFSETOF(LineSplit,ls_str) ==
              COMPILER_OFFSETOF(StringSplit,s_str));
#define linesplit_bool split_bool

PRIVATE struct type_member linesplit_members[] = {
    TYPE_MEMBER_FIELD("__str__",STRUCT_OBJECT,offsetof(LineSplit,ls_str)),
    TYPE_MEMBER_FIELD("__keeplf__",STRUCT_CONST|STRUCT_BOOL,offsetof(LineSplit,ls_keep)),
    TYPE_MEMBER_END
};

INTERN DeeTypeObject DeeLineSplit_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"_linesplit",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FNORMAL|TP_FFINAL,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeSeq_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */NULL,
                /* .tp_copy_ctor = */NULL,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */NULL,
                /* .tp_free      = */NULL,
                {
                    /* .tp_instance_size = */sizeof(LineSplit)
                }
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&linesplit_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */NULL,
        /* .tp_bool = */(int(DCALL *)(DeeObject *__restrict))&linesplit_bool
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */NULL, /* No visit, because it only ever references strings. */
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */NULL,
    /* .tp_seq           = */&linesplit_seq,
    /* .tp_iter_next     = */NULL,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */NULL,
    /* .tp_members       = */linesplit_members,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */linesplit_class_members
};

/* @return: An abstract sequence type for enumerating
 *          the segments of a string split into lines. */
INTERN DREF DeeObject *DCALL
DeeString_SplitLines(DeeObject *__restrict self,
                     bool keepends) {
 DREF LineSplit *result;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 result = DeeObject_MALLOC(LineSplit);
 if unlikely(!result) goto done;
 /* Same as the regular split(), but use the case-insensitive sequence type. */
 DeeObject_Init(result,&DeeLineSplit_Type);
 Dee_Incref(self);
 result->ls_str  = (DREF DeeStringObject *)self; /* Inherit */
 result->ls_keep = keepends;
done:
 return (DREF DeeObject *)result;
}


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_UNICODE_SPLIT_C_INL */
