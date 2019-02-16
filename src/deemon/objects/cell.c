/* Copyright (c) 2019 Griefer@Work                                            *
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
#ifndef GUARD_DEEMON_OBJECTS_CELL_C
#define GUARD_DEEMON_OBJECTS_CELL_C 1
#define _KOS_SOURCE 1

#include <deemon/api.h>
#include <deemon/alloc.h>
#include <deemon/object.h>
#include <deemon/bool.h>
#include <deemon/cell.h>
#include <deemon/gc.h>
#include <deemon/none.h>
#include <deemon/error.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/arg.h>

#include "../runtime/strings.h"

DECL_BEGIN

typedef DeeCellObject Cell;

PUBLIC DREF DeeObject *DCALL
DeeCell_New(DeeObject *__restrict item) {
 DREF Cell *result;
 ASSERT_OBJECT(item);
 result = DeeGCObject_MALLOC(Cell);
 if unlikely(!result) goto done;
 /* Initialize and fill in the new cell. */
 DeeObject_Init(result,&DeeCell_Type);
 Dee_Incref(item);
 result->c_item = item;
#ifndef CONFIG_NO_THREADS
 rwlock_init(&result->c_lock);
#endif /* !CONFIG_NO_THREADS */
 DeeGC_Track((DeeObject *)result);
done:
 return (DREF DeeObject *)result;
}

PRIVATE int DCALL
cell_ctor(Cell *__restrict self) {
 self->c_item = NULL;
#ifndef CONFIG_NO_THREADS
 rwlock_init(&self->c_lock);
#endif /* !CONFIG_NO_THREADS */
 return 0;
}

PRIVATE int DCALL
cell_copy(Cell *__restrict self,
          Cell *__restrict other) {
 DeeCell_LockRead(other);
 self->c_item = other->c_item;
 Dee_XIncref(self->c_item);
 DeeCell_LockEndRead(other);
#ifndef CONFIG_NO_THREADS
 rwlock_init(&self->c_lock);
#endif /* !CONFIG_NO_THREADS */
 return 0;
}

PRIVATE int DCALL
cell_init(Cell *__restrict self,
          size_t argc, DeeObject **__restrict argv) {
 if (DeeArg_Unpack(argc,argv,"o:cell",&self->c_item))
     return -1;
 Dee_Incref(self->c_item);
#ifndef CONFIG_NO_THREADS
 rwlock_init(&self->c_lock);
#endif /* !CONFIG_NO_THREADS */
 return 0;
}

PRIVATE void DCALL
cell_fini(Cell *__restrict self) {
 Dee_XDecref(self->c_item);
}
PRIVATE void DCALL
cell_visit(Cell *__restrict self, dvisit_t proc, void *arg) {
 DeeCell_LockRead(self);
 Dee_XVisit(self->c_item);
 DeeCell_LockEndRead(self);
}
PRIVATE void DCALL
cell_clear(Cell *__restrict self) {
 DREF DeeObject *old_obj;
 DeeCell_LockWrite(self);
 old_obj = self->c_item;
 self->c_item = NULL;
 DeeCell_LockEndWrite(self);
 Dee_XDecref(old_obj);
}

PRIVATE int DCALL
cell_deepload(Cell *__restrict self) {
 return DeeObject_XInplaceDeepCopyWithLock(&self->c_item,
                                           &self->c_lock);
}


PUBLIC DREF DeeObject *DCALL
DeeCell_TryGet(DeeObject *__restrict self) {
 DREF DeeObject *result;
 ASSERT_OBJECT_TYPE(self,&DeeCell_Type);
 DeeCell_LockRead(self);
 result = DeeCell_Item(self);
 Dee_XIncref(result);
 DeeCell_LockEndRead(self);
 return result;
}

PRIVATE void DCALL err_empty_cell(void) {
 DeeError_Throwf(&DeeError_ValueError,
                 "The cell is empty");
}



PUBLIC DREF DeeObject *DCALL
DeeCell_Get(DeeObject *__restrict self) {
 DREF DeeObject *result;
 result = DeeCell_TryGet(self);
 if unlikely(!result) {
  DeeError_Throwf(&DeeError_UnboundAttribute,
                  "The cell is empty");
  /* No mitochondria here... */
 }
 return result;
}
PUBLIC DREF DeeObject *DCALL
DeeCell_Xch(DeeObject *__restrict self,
            DeeObject *value) {
 DREF DeeObject *result;
 ASSERT_OBJECT_TYPE(self,&DeeCell_Type);
 ASSERT_OBJECT_OPT(value);
 /* Exchange the cell's value. */
 Dee_XIncref(value);
 DeeCell_LockWrite(self);
 result = DeeCell_Item(self);
 DeeCell_Item(self) = value;
 DeeCell_LockEndWrite(self);
 return result;
}
PUBLIC DREF DeeObject *DCALL
DeeCell_XchNonNull(DeeObject *__restrict self,
                   DeeObject *value) {
 DREF DeeObject *result;
 ASSERT_OBJECT_TYPE(self,&DeeCell_Type);
 ASSERT_OBJECT_OPT(value);
 /* Exchange the cell's value. */
 Dee_XIncref(value);
 DeeCell_LockWrite(self);
 result = DeeCell_Item(self);
 if unlikely(!result) {
  DeeCell_LockEndWrite(self);
  Dee_DecrefNokill(value);
 } else {
  DeeCell_Item(self) = value;
  DeeCell_LockEndWrite(self);
 }
 return result;
}
PUBLIC DREF DeeObject *DCALL
DeeCell_CmpXch(DeeObject *__restrict self,
               DeeObject *old_value,
               DeeObject *new_value) {
 DREF DeeObject *result;
 ASSERT_OBJECT_TYPE(self,&DeeCell_Type);
 ASSERT_OBJECT_OPT(old_value);
 ASSERT_OBJECT_OPT(new_value);
 /* Extract the cell's value. */
 DeeCell_LockWrite(self);
 result = DeeCell_Item(self);
 if (result == old_value) {
  Dee_XIncref(new_value);
  DeeCell_Item(self) = new_value;
 } else {
  Dee_XIncref(result);
 }
 DeeCell_LockEndWrite(self);
 return result;
}

PUBLIC int DCALL
DeeCell_Del(DeeObject *__restrict self) {
 DREF DeeObject *old_value;
 ASSERT_OBJECT_TYPE(self,&DeeCell_Type);
 /* Extract the cell's value. */
 DeeCell_LockWrite(self);
 old_value = DeeCell_Item(self);
 DeeCell_Item(self) = NULL;
 DeeCell_LockEndWrite(self);
 /* Decref() the old value. */
 Dee_Decref(old_value);
 return 0;
}
PUBLIC int DCALL
DeeCell_Set(DeeObject *__restrict self,
            DeeObject *__restrict value) {
 DREF DeeObject *old_value;
 ASSERT_OBJECT_TYPE(self,&DeeCell_Type);
 ASSERT_OBJECT(value);
 Dee_Incref(value);
 /* Exchange the cell's value. */
 DeeCell_LockWrite(self);
 old_value = DeeCell_Item(self);
 DeeCell_Item(self) = value;
 DeeCell_LockEndWrite(self);
 /* Decref() the old value. */
 Dee_Decref(old_value);
 return 0;
}


PRIVATE DEFINE_STRING(empty_cell_repr,"<>");

PRIVATE DREF DeeObject *DCALL
cell_str(Cell *__restrict self) {
 DREF DeeObject *item;
 item = DeeCell_TryGet((DeeObject *)self);
 if (!item) return_reference_(&str_cell);
 return DeeString_Newf("cell -> %K",item);
}

PRIVATE DREF DeeObject *DCALL
cell_repr(Cell *__restrict self) {
 DREF DeeObject *item;
 item = DeeCell_TryGet((DeeObject *)self);
 if (!item)
      return_reference_((DREF DeeObject *)&empty_cell_repr);
 return DeeString_Newf("<%R>",item);
}

#ifdef CONFIG_NO_THREADS
#define CELL_READITEM(x) DeeCell_Item(x)
#else
#define CELL_READITEM(x) ATOMIC_READ((x)->c_item)
#endif

PRIVATE int DCALL
cell_bool(Cell *__restrict self) {
 return CELL_READITEM(self) != NULL;
}


#define DEFINE_CELL_COMPARE(name,op) \
PRIVATE DREF DeeObject *DCALL \
name(Cell *__restrict self, \
     Cell *__restrict other) { \
 if (DeeObject_AssertType((DeeObject *)other,&DeeCell_Type)) \
     return NULL; \
 return_bool(DeeObject_Id(CELL_READITEM(self)) op \
             DeeObject_Id(CELL_READITEM(other))); \
}
DEFINE_CELL_COMPARE(cell_eq,==)
DEFINE_CELL_COMPARE(cell_ne,!=)
DEFINE_CELL_COMPARE(cell_lo,<)
DEFINE_CELL_COMPARE(cell_le,<=)
DEFINE_CELL_COMPARE(cell_gr,>)
DEFINE_CELL_COMPARE(cell_ge,>=)
#undef DEFINE_CELL_COMPARE

PRIVATE struct type_cmp cell_cmp = {
    /* .tp_hash = */(dhash_t(DCALL *)(DeeObject *__restrict))NULL,
    /* .tp_eq   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&cell_eq,
    /* .tp_ne   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&cell_ne,
    /* .tp_lo   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&cell_lo,
    /* .tp_le   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&cell_le,
    /* .tp_gr   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&cell_gr,
    /* .tp_ge   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&cell_ge
};


PRIVATE struct type_getset cell_getsets[] = {
    { "value", &DeeCell_Get, &DeeCell_Del, &DeeCell_Set,
      DOC("@throw UnboundAttribute Attempted to read from an empty cell\n"
          "read/write access to the underlying, contained :object") },
    { NULL }
};

PRIVATE DREF DeeObject *DCALL
cell_get(Cell *__restrict self,
         size_t argc, DeeObject **__restrict argv) {
 DeeObject *def = NULL,*result;
 if (DeeArg_Unpack(argc,argv,"|o:get",&def))
     goto err;
 result = DeeCell_TryGet((DeeObject *)self);
 if (!result) {
  result = def;
  if (!result)
       goto err_empty;
 }
 return result;
err_empty:
 err_empty_cell();
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
cell_delete(Cell *__restrict self,
            size_t argc, DeeObject **__restrict argv) {
 DeeObject *oldval;
 if (DeeArg_Unpack(argc,argv,":delete"))
     goto err;
 oldval = DeeCell_Xch((DeeObject *)self,NULL);
 if (!oldval) return_false;
 Dee_Decref(oldval);
 return_true;
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
cell_pop(Cell *__restrict self,
         size_t argc, DeeObject **__restrict argv) {
 DeeObject *oldval,*def = NULL;
 if (DeeArg_Unpack(argc,argv,"|o:pop",&def))
     goto err;
 oldval = DeeCell_Xch((DeeObject *)self,NULL);
 if (!oldval) {
  if (def) return_reference_(def);
  goto err_empty;
 }
 return oldval;
err_empty:
 err_empty_cell();
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
cell_set(Cell *__restrict self,
         size_t argc, DeeObject **__restrict argv) {
 DeeObject *newval;
 if (DeeArg_Unpack(argc,argv,"o:set",&newval))
     goto err;
 newval = DeeCell_Xch((DeeObject *)self,newval);
 if (!newval) return_false;
 Dee_Decref(newval);
 return_true;
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
cell_xch(Cell *__restrict self,
         size_t argc, DeeObject **__restrict argv) {
 DeeObject *value,*def = NULL,*result;
 if (DeeArg_Unpack(argc,argv,"o|o:xch",&value,&def))
     goto err;
 if (def) {
  result = DeeCell_Xch((DeeObject *)self,value);
  if (!result) Dee_Incref(def),result = def;
 } else {
  result = DeeCell_XchNonNull((DeeObject *)self,value);
  if (!result)
       goto err_empty;
 }
 return result;
err_empty:
 err_empty_cell();
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
cell_cmpdel(Cell *__restrict self,
            size_t argc, DeeObject **__restrict argv) {
 DeeObject *oldval,*result;
 if (DeeArg_Unpack(argc,argv,"o:cmpdel",&oldval))
     goto err;
 result = DeeCell_CmpXch((DeeObject *)self,oldval,NULL);
 Dee_Decref(result);
 return_bool_(result == oldval);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
cell_cmpxch(Cell *__restrict self,
            size_t argc, DeeObject **__restrict argv) {
 DeeObject *oldval,*newval = NULL,*def = NULL,*result;
 if (DeeArg_Unpack(argc,argv,"o|oo:cmpxch",&oldval,&newval,&def))
     goto err;
 if (newval) {
  result = DeeCell_CmpXch((DeeObject *)self,oldval,newval);
  if (!result) {
   if (!def) goto err_empty;
   Dee_Incref(def);
   result = def;
  }
 } else {
  result = DeeCell_CmpXch((DeeObject *)self,NULL,oldval);
  if (!result) return_true;
  Dee_Incref(Dee_False);
  Dee_Decref(result);
  result = Dee_False;
 }
 return result;
err_empty:
 err_empty_cell();
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
cell_cmpset(Cell *__restrict self,
            size_t argc, DeeObject **__restrict argv) {
 DeeObject *oldval,*newval = NULL,*result;
 if (DeeArg_Unpack(argc,argv,"o|o:cmpset",&oldval,&newval))
     goto err;
 result = DeeCell_CmpXch((DeeObject *)self,oldval,newval);
 Dee_XDecref(result);
 return_bool_(result == oldval);
err:
 return NULL;
}


PRIVATE struct type_method cell_methods[] = {
    { DeeString_STR(&str_get),
     (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&cell_get,
      DOC("->\n"
          "@throw ValueError @this cell is empty\n"
          "Returns the contained value of the cell\n"
          "\n"
          "(def)->\n"
          "Returns the contained value of the cell or @def when it is empty") },
    { "delete",
     (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&cell_delete,
      DOC("->?Dbool\n"
          "Delete the value stored in @this cell, returning :true if "
          "the cell wasn't empty before, or :false if it already was") },
    { DeeString_STR(&str_pop),
     (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&cell_pop,
      DOC("->\n"
          "@throw ValueError The cell was empty\n"
          "\n"
          "(def)->\n"
          "Pop and return the previously contained object, @def, or throw a ValueError") },
    { DeeString_STR(&str_set),
     (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&cell_set,
      DOC("(value)->?Dbool\n"
          "Set (override) @this cell's value, returning :true if a previous value "
          "has been overwritten, or :falue if no value had been set before") },
    { DeeString_STR(&str_xch),
     (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&cell_xch,
      DOC("(value)->\n"
          "@throw ValueError @this cell is empty\n"
          "Overwrite the cell's value and return the old value or throw an error when it was empty\n"
          "\n"
          "(value,def)->\n"
          "Returns the contained value of the cell or @def when it is empty") },
    { "cmpdel",
     (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&cell_cmpdel,
      DOC("(old_value)->?Dbool\n"
          "Atomically check if the stored object's id matches @{old_value}. If this is "
          "the case, delete the stored object and return :{true}. Otherwise, return :false") },
    { "cmpxch",
     (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&cell_cmpxch,
      DOC("(old_value,new_value)->\n"
          "@throw ValueError @this cell is empty\n"
          "\n"
          "(old_value,new_value,def)->\n"
          "Do an id-compare of the stored object against @old_value and overwrite "
          "that object with @new_value when they match. Otherwise, don't modify the "
          "stored object. In both cases, return the previously stored object, @def, or throw a :{ValueError}.\n"
          "This is equivalent to the atomic execution of the following:\n"
          ">local result = this.old_value;\n"
          ">if (this && result === old_value)\n"
          ">    this.value = new_value;\n"
          ">return result\n"
          "\n"
          "(new_value)->?Dbool\n"
          "Return :true and atomically set @new_value as stored object only "
          "if no object had been set before. Otherwise, return :false") },
    { "cmpset",
     (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&cell_cmpset,
      DOC("(old_value)->?Dbool\n"
          "(old_value,new_value)->?Dbool\n"
          "Atomically check if the stored value equals @old_value and return :true "
          "alongside storing @new_value if this is the case. Otherwise, return :false\n"
          "When @new_value is omit, the function behaves identical to #cmpdel") },
#ifndef CONFIG_NO_DEEMON_100_COMPAT
    { "del",
     (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&cell_delete,
      DOC("->?Dbool\n"
          "Deprecated alias for #delete") },
    { "exchange",
     (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&cell_xch,
      DOC("(value)->\n"
          "(value,def)->\n"
          "Deprecated alias for #xch") },
#endif /* !CONFIG_NO_DEEMON_100_COMPAT */
    { NULL }
};

PRIVATE struct type_gc cell_gc = {
    /* .tp_clear = */(void(DCALL *)(DeeObject *__restrict))&cell_clear
};

PUBLIC DeeTypeObject DeeCell_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */DeeString_STR(&str_cell),
    /* .tp_doc      = */DOC("A 1-layer indirection allowing for referral to another object\n"
                            "\n"
                            "()\n"
                            "Create a new, empty cell\n"
                            "\n"
                            "(obj)\n"
                            "Create a new cell containing @obj"),
    /* .tp_flags    = */TP_FNORMAL|TP_FGC|TP_FNAMEOBJECT,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeObject_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */(void *)&cell_ctor,
                /* .tp_copy_ctor = */(void *)&cell_copy,
                /* .tp_deep_ctor = */(void *)&cell_copy,
                /* .tp_any_ctor  = */(void *)&cell_init,
                TYPE_FIXED_ALLOCATOR_GC(Cell)
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&cell_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL,
        /* .tp_deepload    = */(int(DCALL *)(DeeObject *__restrict))&cell_deepload
    },
    /* .tp_cast = */{
        /* .tp_str  = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&cell_str,
        /* .tp_repr = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&cell_repr,
        /* .tp_bool = */(int(DCALL *)(DeeObject *__restrict))&cell_bool
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */(void(DCALL *)(DeeObject *__restrict,dvisit_t,void *))&cell_visit,
    /* .tp_gc            = */&cell_gc,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */&cell_cmp,
    /* .tp_seq           = */NULL,
    /* .tp_iter_next     = */NULL,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */cell_methods,
    /* .tp_getsets       = */cell_getsets,
    /* .tp_members       = */NULL,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */NULL
};

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_CELL_C */
