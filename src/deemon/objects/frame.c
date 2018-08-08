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
#ifndef GUARD_DEEMON_OBJECTS_FRAME_C
#define GUARD_DEEMON_OBJECTS_FRAME_C 1

#include <deemon/api.h>
#include <deemon/object.h>
#include <deemon/code.h>
#include <deemon/traceback.h>
#include <deemon/seq.h>
#include <deemon/none.h>
#include <deemon/int.h>
#include <deemon/gc.h>
#include <deemon/string.h>
#include <deemon/thread.h>

#include "../runtime/strings.h"
#include "../runtime/runtime_error.h"

#include <stddef.h>
#include <stdarg.h>
#include <string.h>

DECL_BEGIN

typedef DeeFrameObject Frame;

#ifndef CONFIG_NO_THREADS
#define PLOCK_READ(x) \
    ((x)->f_plock ? \
    ((x)->f_flags & DEEFRAME_FRECLOCK ? (void)recursive_rwlock_read((x)->f_prlock) \
                                      : (void)rwlock_read((x)->f_plock)) : (void)0)
#define PLOCK_TRYREAD(x) \
    ((x)->f_plock ? \
    ((x)->f_flags & DEEFRAME_FRECLOCK ? recursive_rwlock_tryread((x)->f_prlock) \
                                      : rwlock_tryread((x)->f_plock)) : true)
#define PLOCK_WRITE(x) \
    ((x)->f_flags & DEEFRAME_FWRITABLE ? ((x)->f_plock ? \
    ((x)->f_flags & DEEFRAME_FRECLOCK ? (recursive_rwlock_write((x)->f_prlock),true) \
                                      : (rwlock_write((x)->f_plock)),true) : true) : false)
#define PLOCK_TRYWRITE(x) \
    ((x)->f_flags & DEEFRAME_FWRITABLE ? ((x)->f_plock ? \
    ((x)->f_flags & DEEFRAME_FRECLOCK ? recursive_rwlock_trywrite((x)->f_prlock) \
                                      : rwlock_trywrite((x)->f_plock)) : true) : false)
#define PLOCK_ENDREAD(x) \
    ((x)->f_plock ? \
    ((x)->f_flags & DEEFRAME_FRECLOCK ? (void)recursive_rwlock_endread((x)->f_prlock) \
                                      : (void)rwlock_endread((x)->f_plock)) : (void)0)
#define PLOCK_ENDWRITE(x) \
    ((x)->f_plock ? \
    ((x)->f_flags & DEEFRAME_FRECLOCK ? (void)recursive_rwlock_endwrite((x)->f_prlock) \
                                      : (void)rwlock_endwrite((x)->f_plock)) : (void)0)
#else
#define PLOCK_READ(x)     (void)0
#define PLOCK_TRYREAD(x)   true
#define PLOCK_WRITE(x)    ((x)->f_flags & DEEFRAME_FWRITABLE)
#define PLOCK_TRYWRITE(x) ((x)->f_flags & DEEFRAME_FWRITABLE)
#define PLOCK_ENDREAD(x)  (void)0
#define PLOCK_ENDWRITE(x) (void)0
#endif

#ifndef CONFIG_NO_THREADS
PUBLIC DREF DeeObject *
(DCALL DeeFrame_NewReferenceWithLock)(DeeObject *owner,
                                      struct code_frame *__restrict frame,
                                      uint16_t flags, void *lock)
#else
PUBLIC DREF DeeObject *
(DCALL DeeFrame_NewReference)(DeeObject *owner,
                              struct code_frame *__restrict frame,
                              uint16_t flags)
#endif
{
 DREF Frame *result;
 ASSERT_OBJECT_OPT(owner);
 result = DeeObject_MALLOC(Frame);
 if unlikely(!result) goto done;
 result->f_owner = owner;
 result->f_frame = frame;
 result->f_flags = flags;
#ifndef CONFIG_NO_THREADS
 rwlock_init(&result->f_lock);
 result->f_plock = (rwlock_t *)lock;
#endif
 Dee_XIncref(owner);
 DeeObject_Init(result,&DeeFrame_Type);
done:
 return (DREF DeeObject *)result;
}


PUBLIC void DCALL
DeeFrame_DecrefShared(DREF DeeObject *__restrict self) {
 Frame *me;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeFrame_Type);
 me = (Frame *)self;
 rwlock_write(&me->f_lock);
 me->f_frame = NULL;
 rwlock_endwrite(&me->f_lock);
 Dee_Decref_likely(self);
}



#if defined(CONFIG_HOST_WINDOWS) && 0
#define TRACEBACK_SLASH  "\\"
#else
#define TRACEBACK_SLASH  "/"
#endif

INTERN dssize_t DCALL
print_ddi(struct ascii_printer *__restrict printer,
          DeeCodeObject *__restrict code, code_addr_t ip) {
 dssize_t print_error;
 DeeDDIObject *ddi = code->co_ddi;
 struct ddi_state state;
 if (!DeeCode_FindDDI((DeeObject *)code,&state,NULL,ip,DDI_STATE_FNOTHROW|DDI_STATE_FNONAMES)) {
  print_error = ascii_printer_printf(printer,"%s+%.4I32X\n",DeeDDI_NAME(ddi),ip);
 } else {
  struct ddi_xregs *iter;
  char const *path,*file,*name;
  char const *base_name = DeeDDI_NAME(ddi);
  DDI_STATE_DO(iter,&state) {
   file = DeeDDI_VALID_FILE(ddi,iter->dx_base.dr_file) ? DeeDDI_FILE_NAME(ddi,iter->dx_base.dr_file) : "";
   name = DeeDDI_VALID_SYMBOL(ddi,iter->dx_base.dr_name) ? DeeDDI_SYMBOL_NAME(ddi,iter->dx_base.dr_name) : base_name;
   if (!state.rs_regs.dr_path--) path = "";
   else path = DeeDDI_HAS_PATH(ddi,state.rs_regs.dr_path) ? DeeDDI_PATH_NAME(ddi,state.rs_regs.dr_path) : "";
   print_error = ascii_printer_printf(printer,
                                      "%s%s%s(%d,%d) : %s+%.4I32X",
                                      path,*path ? TRACEBACK_SLASH : "",file,
                                      iter->dx_base.dr_lno+1,
                                      iter->dx_base.dr_col+1,
                                      name,ip);
   if unlikely(print_error < 0) break;
   if (name != base_name && *base_name) {
    /* Also print the name of the base-function */
    print_error = ascii_printer_printf(printer," (%s)",base_name);
    if unlikely(print_error < 0) break;
   }
   print_error = ascii_printer_putc(printer,'\n');
   if unlikely(print_error < 0) break;
  }
  DDI_STATE_WHILE(iter,&state);
  ddi_state_fini(&state);
 }
 return print_error;
}

PRIVATE DREF DeeObject *DCALL
print_ddi_string(DeeCodeObject *__restrict code, code_addr_t ip) {
 struct ascii_printer printer = ASCII_PRINTER_INIT;
 if unlikely(print_ddi(&printer,code,ip) < 0) goto err;
 return ascii_printer_pack(&printer);
err:
 ascii_printer_fini(&printer);
 return NULL;
}



PRIVATE void DCALL
frame_fini(Frame *__restrict self) {
 Dee_XDecref(self->f_owner);
}
PRIVATE void DCALL
frame_visit(Frame *__restrict self, dvisit_t proc, void *arg) {
 Dee_XVisit(self->f_owner);
}
PRIVATE DREF DeeObject *DCALL
frame_repr(Frame *__restrict self) {
 DREF DeeCodeObject *code;
 DREF DeeObject *result;
 code_addr_t ip;
 rwlock_read(&self->f_lock);
 if (!self->f_frame) {
  rwlock_endread(&self->f_lock);
  return_empty_string;
 }
 PLOCK_READ(self);
 code = self->f_frame->cf_func->fo_code;
 Dee_Incref(code);
 ip = (code_addr_t)(self->f_frame->cf_ip-code->co_code);
 PLOCK_ENDREAD(self);
 rwlock_endread(&self->f_lock);
 result = print_ddi_string(code,ip);
 Dee_Decref(code);
 return result;
}

PRIVATE DREF DeeCodeObject *DCALL
frame_getddi(Frame *__restrict self,
             struct ddi_state *__restrict state,
             code_addr_t *pstartip,
             code_addr_t *pendip,
             unsigned int flags) {
 uint8_t *result; code_addr_t startip;
 DREF DeeCodeObject *code;
 rwlock_read(&self->f_lock);
 if unlikely(!self->f_frame) {
  rwlock_endread(&self->f_lock);
  return (DREF DeeCodeObject *)ITER_DONE;
 }
 PLOCK_READ(self);
 code = self->f_frame->cf_func->fo_code;
 Dee_Incref(code);
 startip = (code_addr_t)(self->f_frame->cf_ip -
                         code->co_code);
 PLOCK_ENDREAD(self);
 rwlock_endread(&self->f_lock);
 if (pstartip) *pstartip = startip;
 result = DeeCode_FindDDI((DeeObject *)code,
                           state,
                           pendip,
                           startip,
                           flags);
 if (DDI_ISOK(result))
     return code;
 Dee_Decref(code);
 return result == DDI_NEXT_DONE
      ? (DREF DeeCodeObject *)ITER_DONE
      : (DREF DeeCodeObject *)NULL;
}

PRIVATE DREF DeeObject *DCALL
frame_getfile(Frame *__restrict self) {
 DREF DeeObject *result;
 DREF DeeCodeObject *code;
 struct ddi_state state;
 char const *path = "",*file;
 code = frame_getddi(self,&state,NULL,NULL,DDI_STATE_FNONAMES);
 if unlikely(!code) return NULL;
 if (code == (DREF DeeCodeObject *)ITER_DONE)
     return_empty_string;
 file = DeeDDI_VALID_FILE(code->co_ddi,state.rs_regs.dr_file)
      ? DeeDDI_FILE_NAME(code->co_ddi,state.rs_regs.dr_file) : "";
 if (!state.rs_regs.dr_path--)
  ;
 else if (DeeDDI_VALID_PATH(code->co_ddi,state.rs_regs.dr_path))
  path = DeeDDI_PATH_NAME(code->co_ddi,state.rs_regs.dr_path);
 if (!*path)
  result = DeeString_New(file);
 else {
  result = DeeString_Newf("%s" TRACEBACK_SLASH "%s",path,file);
 }
 ddi_state_fini(&state);
 Dee_Decref(code);
 return result;
}

PRIVATE DREF DeeObject *DCALL
frame_getline(Frame *__restrict self) {
 DREF DeeObject *result;
 DREF DeeCodeObject *code;
 struct ddi_state state;
 code = frame_getddi(self,&state,NULL,NULL,DDI_STATE_FNONAMES);
 if unlikely(!code) return NULL;
 if (code == (DREF DeeCodeObject *)ITER_DONE)
     return_reference_(&DeeInt_MinusOne);
 result = DeeInt_NewInt(state.rs_regs.dr_lno);
 ddi_state_fini(&state);
 Dee_Decref(code);
 return result;
}

PRIVATE DREF DeeObject *DCALL
frame_getcol(Frame *__restrict self) {
 DREF DeeObject *result;
 DREF DeeCodeObject *code;
 struct ddi_state state;
 code = frame_getddi(self,&state,NULL,NULL,DDI_STATE_FNONAMES);
 if unlikely(!code) return NULL;
 if (code == (DREF DeeCodeObject *)ITER_DONE)
     return_reference_(&DeeInt_MinusOne);
 result = DeeInt_NewInt(state.rs_regs.dr_col);
 ddi_state_fini(&state);
 Dee_Decref(code);
 return result;
}

PRIVATE DREF DeeObject *DCALL
frame_getname(Frame *__restrict self) {
 DREF DeeObject *result;
 DREF DeeCodeObject *code;
 struct ddi_state state;
 char const *name;
 code = frame_getddi(self,&state,NULL,NULL,DDI_STATE_FNONAMES);
 if unlikely(!code) return NULL;
 if (code == (DREF DeeCodeObject *)ITER_DONE)
     return_empty_string;
 name = DeeDDI_VALID_SYMBOL(code->co_ddi,state.rs_regs.dr_file)
      ? DeeDDI_SYMBOL_NAME(code->co_ddi,state.rs_regs.dr_file) : "";
 result = DeeString_New(name);
 ddi_state_fini(&state);
 Dee_Decref(code);
 return result;
}

PRIVATE struct type_getset frame_getsets[] = {
    { "file", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&frame_getfile, NULL, NULL,
      DOC("->string\nThe filename of this frame's source file, or an empty string") },
    { "line", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&frame_getline, NULL, NULL,
      DOC("->int\nThe line number within this frame's source file, or ${-1}") },
    { "col", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&frame_getcol, NULL, NULL,
      DOC("->int\nThe column offset within this frame's source file, or ${-1}") },
    { "name", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&frame_getname, NULL, NULL,
      DOC("->string\nThe name of this frame's function, or an empty string") },
    { NULL }
};

PUBLIC DeeTypeObject DeeFrame_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */DeeString_STR(&str_frame),
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FNORMAL|TP_FFINAL,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeObject_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */NULL,
                /* .tp_copy_ctor = */NULL,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */NULL,
                /* .tp_free      = */NULL,
                {
                    /* .tp_instance_size = */sizeof(Frame)
                }
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&frame_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&frame_repr,
        /* .tp_bool = */NULL
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */(void(DCALL *)(DeeObject *__restrict,dvisit_t,void*))&frame_visit,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */NULL,
    /* .tp_seq           = */NULL,
    /* .tp_iter_next     = */NULL,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */frame_getsets,
    /* .tp_members       = */NULL,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */NULL
};

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_FRAME_C */
