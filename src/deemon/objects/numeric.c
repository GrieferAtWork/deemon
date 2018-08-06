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
#ifndef GUARD_DEEMON_OBJECTS_NUMERIC_C
#define GUARD_DEEMON_OBJECTS_NUMERIC_C 1

#include <deemon/api.h>
#include <deemon/object.h>
#include <deemon/numeric.h>
#include <deemon/string.h>
#include <deemon/int.h>

#include "../runtime/strings.h"

DECL_BEGIN

INTDEF int DCALL none_i1(void *UNUSED(b));
INTDEF int DCALL none_i2(void *UNUSED(b), void *UNUSED(c));

PRIVATE DREF DeeObject *DCALL
numeric_asi8(DeeObject *__restrict self) {
 int8_t result;
 if (DeeObject_AsInt8(self,&result))
     return NULL;
 return DeeInt_NewS8(result);
}
PRIVATE DREF DeeObject *DCALL
numeric_asi16(DeeObject *__restrict self) {
 int16_t result;
 if (DeeObject_AsInt16(self,&result))
     return NULL;
 return DeeInt_NewS16(result);
}
PRIVATE DREF DeeObject *DCALL
numeric_asi32(DeeObject *__restrict self) {
 int32_t result;
 if (DeeObject_AsInt32(self,&result))
     return NULL;
 return DeeInt_NewS32(result);
}
PRIVATE DREF DeeObject *DCALL
numeric_asi64(DeeObject *__restrict self) {
 int64_t result;
 if (DeeObject_AsInt64(self,&result))
     return NULL;
 return DeeInt_NewS64(result);
}
PRIVATE DREF DeeObject *DCALL
numeric_asu8(DeeObject *__restrict self) {
 uint8_t result;
 if (DeeObject_AsUInt8(self,&result))
     return NULL;
 return DeeInt_NewU8(result);
}
PRIVATE DREF DeeObject *DCALL
numeric_asu16(DeeObject *__restrict self) {
 uint16_t result;
 if (DeeObject_AsUInt16(self,&result))
     return NULL;
 return DeeInt_NewU16(result);
}
PRIVATE DREF DeeObject *DCALL
numeric_asu32(DeeObject *__restrict self) {
 uint32_t result;
 if (DeeObject_AsUInt32(self,&result))
     return NULL;
 return DeeInt_NewU32(result);
}
PRIVATE DREF DeeObject *DCALL
numeric_asu64(DeeObject *__restrict self) {
 uint64_t result;
 if (DeeObject_AsUInt64(self,&result))
     return NULL;
 return DeeInt_NewU64(result);
}

PRIVATE struct type_getset numeric_getsets[] = {
    { "asint", &DeeObject_Int, NULL, NULL,
      DOC("->int\n"
          "@throw NotImplemented @this number does not implement ${operator int}\n"
          "Return @this number as an integer, truncating all digits after a dot/comma") },
    { "asi8", &numeric_asi8, NULL, NULL,
      DOC("->int\n"
          "@throw NotImplemented @this number does not implement ${operator int}\n"
          "@throw IntegerOverflow The value of @this number is outside the requested range\n"
          "Return @this number as an integer in the range of ${-128 ... 127}") },
    { "asi16", &numeric_asi16, NULL, NULL,
      DOC("->int\n"
          "@throw NotImplemented @this number does not implement ${operator int}\n"
          "@throw IntegerOverflow The value of @this number is outside the requested range\n"
          "Return @this number as an integer in the range of ${-32768 ... 32767}") },
    { "asi32", &numeric_asi32, NULL, NULL,
      DOC("->int\n"
          "@throw NotImplemented @this number does not implement ${operator int}\n"
          "@throw IntegerOverflow The value of @this number is outside the requested range\n"
          "Return @this number as an integer in the range of ${-2147483648 ... 2147483647}") },
    { "asi64", &numeric_asi64, NULL, NULL,
      DOC("->int\n"
          "@throw NotImplemented @this number does not implement ${operator int}\n"
          "@throw IntegerOverflow The value of @this number is outside the requested range\n"
          "Return @this number as an integer in the range of ${-9223372036854775808 ... 9223372036854775807}") },
    { "asu8", &numeric_asu8, NULL, NULL,
      DOC("->int\n"
          "@throw NotImplemented @this number does not implement ${operator int}\n"
          "@throw IntegerOverflow The value of @this number is outside the requested range\n"
          "Return @this number as an integer in the range of ${0x0 ... 0xff}") },
    { "asu16", &numeric_asu16, NULL, NULL,
      DOC("->int\n"
          "@throw NotImplemented @this number does not implement ${operator int}\n"
          "@throw IntegerOverflow The value of @this number is outside the requested range\n"
          "Return @this number as an integer in the range of ${0x0 ... 0xffff}") },
    { "asu32", &numeric_asu32, NULL, NULL,
      DOC("->int\n"
          "@throw NotImplemented @this number does not implement ${operator int}\n"
          "@throw IntegerOverflow The value of @this number is outside the requested range\n"
          "Return @this number as an integer in the range of ${0x0 ... 0xffffffff}") },
    { "asu64", &numeric_asu64, NULL, NULL,
      DOC("->int\n"
          "@throw NotImplemented @this number does not implement ${operator int}\n"
          "@throw IntegerOverflow The value of @this number is outside the requested range\n"
          "Return @this number as an integer in the range of ${0x0 ... 0xffffffffffffffff}") },
};


PUBLIC DeeTypeObject DeeNumeric_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */DeeString_STR(&str_numeric),
    /* .tp_doc      = */DOC("Base class for :int and :float"),
    /* .tp_flags    = */TP_FNORMAL,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeObject_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */&none_i1,
                /* .tp_copy_ctor = */&none_i2,
                /* .tp_deep_ctor = */&none_i2,
                /* .tp_any_ctor  = */NULL,
                /* .tp_free      = */NULL,
                {
                    /* .tp_instance_size = */sizeof(DeeObject)
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
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */NULL,
    /* .tp_seq           = */NULL,
    /* .tp_iter_next     = */NULL,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */numeric_getsets,
    /* .tp_members       = */NULL,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */NULL
};

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_NUMERIC_C */
