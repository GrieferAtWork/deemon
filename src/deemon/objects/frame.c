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
#ifndef GUARD_DEEMON_OBJECTS_FRAME_C
#define GUARD_DEEMON_OBJECTS_FRAME_C 1

#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/alloc.h>
#include <deemon/object.h>
#include <deemon/code.h>
#include <deemon/traceback.h>
#include <deemon/seq.h>
#include <deemon/none.h>
#include <deemon/int.h>
#include <deemon/bool.h>
#include <deemon/error.h>
#include <deemon/tuple.h>
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
 struct ddi_state state;
 if (!DeeCode_FindDDI((DeeObject *)code,&state,NULL,ip,DDI_STATE_FNOTHROW|DDI_STATE_FNONAMES)) {
  print_error = ascii_printer_printf(printer,
                                     "%s+%.4I32X\n",
                                     DeeCode_NAME(code),
                                     ip);
 } else {
  struct ddi_xregs *iter;
  char const *path,*file,*name;
  char const *base_name = DeeCode_NAME(code);
  DDI_STATE_DO(iter,&state) {
   file = DeeCode_GetDDIString((DeeObject *)code,iter->dx_base.dr_file);
   name = DeeCode_GetDDIString((DeeObject *)code,iter->dx_base.dr_name);
   if (!state.rs_regs.dr_path--) path = NULL;
   else path = DeeCode_GetDDIString((DeeObject *)code,iter->dx_base.dr_path);
   print_error = ascii_printer_printf(printer,
                                      "%s%s%s(%d,%d) : %s+%.4I32X",
                                      path ? path : "",
                                      path ? TRACEBACK_SLASH : "",
                                      file ? file : "",
                                      iter->dx_base.dr_lno+1,
                                      iter->dx_base.dr_col+1,
                                      name ? name : (code->co_flags & CODE_FCONSTRUCTOR
                                           ? "<anonymous_ctor>"
                                           : "<anonymous>"),
                                      ip);
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
  Dee_ddi_state_fini(&state);
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
frame_getlocation(Frame *__restrict self) {
 DREF DeeObject *result,*entry;
 DREF DeeObject *fileob,*nameob;
 DREF DeeCodeObject *code;
 struct ddi_xregs *iter;
 struct ddi_state state;
 size_t i,count;
 code = frame_getddi(self,&state,NULL,NULL,DDI_STATE_FNONAMES);
 if unlikely(!code) goto err;
 if unlikely(code == (DREF DeeCodeObject *)ITER_DONE)
    return DeeTuple_Newf("(nnnn)");
 count = 0;
 DDI_STATE_DO(iter,&state) {
  ++count;
 }
 DDI_STATE_WHILE(iter,&state);
 result = DeeTuple_NewUninitialized(count);
 if unlikely(!result)
    goto err_state;
 i = 0;
 DDI_STATE_DO(iter,&state) {
  char *temp,*file;
  file = DeeCode_GetDDIString((DeeObject *)code,state.rs_regs.dr_file);
  if unlikely(!file)
     fileob = Dee_None,Dee_Incref(Dee_None);
  else {
   if (!state.rs_regs.dr_path-- ||
      (temp = DeeCode_GetDDIString((DeeObject *)code,state.rs_regs.dr_file)) == NULL)
    fileob = DeeString_New(file);
   else {
    fileob = DeeString_Newf("%s" TRACEBACK_SLASH "%s",temp,file);
   }
   if unlikely(!fileob)
      goto err_state_r;
  }
  temp = DeeCode_GetDDIString((DeeObject *)code,state.rs_regs.dr_name);
  if (!temp)
      nameob = Dee_None,Dee_Incref(Dee_None);
  else {
   nameob = DeeString_New(temp);
   if unlikely(!nameob)
      goto err_state_r_fileob;
  }
  entry = DeeTuple_Newf("oddo",
                        fileob,
                        state.rs_regs.dr_lno,
                        state.rs_regs.dr_col,
                        nameob);
  Dee_Decref(nameob);
  Dee_Decref(fileob);
  if unlikely(!entry) goto err_state_r;
  DeeTuple_SET(result,i,entry); /* Inherit reference. */
  ++i;
 }
 DDI_STATE_WHILE(iter,&state);
 ASSERT(i == count);
 Dee_ddi_state_fini(&state);
 Dee_Decref(code);
 return result;
err_state_r_fileob:
 Dee_Decref(fileob);
err_state_r:
 while (i--)
     Dee_Decref(DeeTuple_GET(result,i));
 DeeTuple_FreeUninitialized(result);
err_state:
 Dee_ddi_state_fini(&state);
 Dee_Decref(code);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
frame_getfile(Frame *__restrict self) {
 DREF DeeObject *result;
 DREF DeeCodeObject *code;
 struct ddi_state state;
 char *temp,*file;
 code = frame_getddi(self,&state,NULL,NULL,DDI_STATE_FNONAMES);
 if unlikely(!code) goto err;
 file = DeeCode_GetDDIString((DeeObject *)code,state.rs_regs.dr_file);
 if unlikely(!file)
    result = Dee_None,Dee_Incref(Dee_None);
 else {
  if (!state.rs_regs.dr_path-- ||
     (temp = DeeCode_GetDDIString((DeeObject *)code,state.rs_regs.dr_file)) == NULL)
   result = DeeString_New(file);
  else {
   result = DeeString_Newf("%s" TRACEBACK_SLASH "%s",temp,file);
  }
 }
 Dee_ddi_state_fini(&state);
 Dee_Decref(code);
 return result;
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
frame_getline(Frame *__restrict self) {
 DREF DeeObject *result;
 DREF DeeCodeObject *code;
 struct ddi_state state;
 code = frame_getddi(self,&state,NULL,NULL,DDI_STATE_FNONAMES);
 if unlikely(!code) goto err;
 if (code == (DREF DeeCodeObject *)ITER_DONE)
     return_none;
 result = DeeInt_NewInt(state.rs_regs.dr_lno);
 Dee_ddi_state_fini(&state);
 Dee_Decref(code);
 return result;
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
frame_getcol(Frame *__restrict self) {
 DREF DeeObject *result;
 DREF DeeCodeObject *code;
 struct ddi_state state;
 code = frame_getddi(self,&state,NULL,NULL,DDI_STATE_FNONAMES);
 if unlikely(!code) goto err;
 if (code == (DREF DeeCodeObject *)ITER_DONE)
     return_none;
 result = DeeInt_NewInt(state.rs_regs.dr_col);
 Dee_ddi_state_fini(&state);
 Dee_Decref(code);
 return result;
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
frame_getname(Frame *__restrict self) {
 DREF DeeObject *result;
 DREF DeeCodeObject *code;
 struct ddi_state state; char *name;
 code = frame_getddi(self,&state,NULL,NULL,DDI_STATE_FNONAMES);
 if unlikely(!code) goto err;
 if (code == (DREF DeeCodeObject *)ITER_DONE ||
    (name = DeeCode_GetDDIString((DeeObject *)code,state.rs_regs.dr_name)) == NULL)
     return_none;
 result = DeeString_New(name);
 Dee_ddi_state_fini(&state);
 Dee_Decref(code);
 return result;
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
frame_getfunc(Frame *__restrict self) {
 DREF DeeFunctionObject *result;
 rwlock_read(&self->f_lock);
 if unlikely(!self->f_frame) {
  rwlock_endread(&self->f_lock);
  return_none;
 }
 PLOCK_READ(self);
 result = self->f_frame->cf_func;
 Dee_Incref(result);
 PLOCK_ENDREAD(self);
 rwlock_endread(&self->f_lock);
 return (DREF DeeObject *)result;
}

PRIVATE DREF DeeObject *DCALL
frame_iswritable(Frame *__restrict self) {
 return_bool(self->f_flags & DEEFRAME_FWRITABLE);
}

PRIVATE ATTR_COLD int DCALL
err_dead_frame(Frame *__restrict UNUSED(self)) {
 return DeeError_Throwf(&DeeError_ReferenceError,
                        "Frame access was revoked");
}
PRIVATE ATTR_COLD int DCALL
err_readonly_frame(Frame *__restrict UNUSED(self)) {
 return DeeError_Throwf(&DeeError_ValueError,
                        "The frame is readonly and cannot be modifed");
}

PRIVATE DREF DeeObject *DCALL
frame_getcode(Frame *__restrict self) {
 DREF DeeCodeObject *result;
 rwlock_read(&self->f_lock);
 if unlikely(!self->f_frame)
    goto err_df;
 PLOCK_READ(self);
 result = self->f_frame->cf_func->fo_code;
 Dee_Incref(result);
 PLOCK_ENDREAD(self);
 rwlock_endread(&self->f_lock);
 return (DREF DeeObject *)result;
err_df:
 rwlock_endread(&self->f_lock);
 err_dead_frame(self);
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
frame_getpc(Frame *__restrict self) {
 code_addr_t pc;
 rwlock_read(&self->f_lock);
 if unlikely(!self->f_frame)
    goto err_df;
 PLOCK_READ(self);
 pc = (code_addr_t)(self->f_frame->cf_ip -
                    self->f_frame->cf_func->fo_code->co_code);
 PLOCK_ENDREAD(self);
 rwlock_endread(&self->f_lock);
 return DeeInt_NewU32(pc);
err_df:
 rwlock_endread(&self->f_lock);
 err_dead_frame(self);
 return NULL;
}

/* Try to reverse-engineer SP based on other information
 * @return: * : The actual depth of the stack. (`DEEFRAME_FUNDEFSP' was unset, and `f_frame->cf_sp' was updated)
 *              NOTE: If the `DEEFRAME_FUNDEFSP' was already unset, don't unset it again
 *                    and discard all reversed information before returning whatever was
 *                    stored within `f_frame->cf_sp' at that point. (must be done to prevent a race condition)
 * @return: -1: Information could not be determined. (no error was thrown, but `DEEFRAME_FUNDEFSP2' was set)
 * @return: -2: The frame has continued execution, or was otherwise released. (no error was thrown) */
PRIVATE int32_t DCALL
frame_revengsp(Frame *__restrict self) {
 (void)self;
 /* TODO */
 return -1;
}

/* Similar to `frame_revengsp()', but instead of always trying to reverse
 * the stack depth, check if the depth was already know upon entry, returning
 * the stored depth when `DEEFRAME_FUNDEFSP' isn't set, or always returning
 * `-1' when `DEEFRAME_FUNDEFSP2' is set.
 * @return: * : The actual depth of the stack.
 * @return: -1: Information could not be determined. (no error was thrown)
 * @return: -2: The frame has continued execution, or was otherwise released. (no error was thrown) */
PRIVATE int32_t DCALL
frame_getsp(Frame *__restrict self) {
 int32_t result;
 uint16_t flags = ATOMIC_READ(self->f_flags);
 if (!(flags & DEEFRAME_FUNDEFSP)) {
  rwlock_read(&self->f_lock);
  if unlikely(!self->f_frame) {
   rwlock_endread(&self->f_lock);
   return -2;
  }
  if (flags & DEEFRAME_FREGENGSP) {
   result = self->f_revsp;
  } else {
   PLOCK_READ(self);
   result = (int32_t)(self->f_frame->cf_sp -
                      self->f_frame->cf_stack);
   PLOCK_ENDREAD(self);
  }
  rwlock_endread(&self->f_lock);
  return result;
 }
 if (flags & DEEFRAME_FUNDEFSP2)
     return -1;
 /* Try to reverse-engineer stack information. */
 return frame_revengsp(self);
}

PRIVATE int DCALL
frame_setpc(Frame *__restrict self, DeeObject *__restrict value) {
 code_addr_t pc;
 if unlikely(!(self->f_flags & DEEFRAME_FWRITABLE))
    return err_readonly_frame(self);
 if unlikely(DeeObject_AsUInt32(value,&pc))
    goto err;
 /* Make sure that the stack-depth is either entirely unknown,
  * or has been reverse engineered based on the starting PC.
  * This is required, since we're about to modify PC, meaning that
  * if SP was still unknown at this point, trying to reverse it at
  * a later point in time could yield invalid results. */
 if ((self->f_flags & (DEEFRAME_FUNDEFSP|DEEFRAME_FUNDEFSP2)) ==
                       DEEFRAME_FUNDEFSP)
      frame_revengsp(self);
 rwlock_read(&self->f_lock);
 if unlikely(!self->f_frame)
    goto err_df;
 PLOCK_READ(self);
 self->f_frame->cf_ip = self->f_frame->cf_func->fo_code->co_code + pc;
 PLOCK_ENDREAD(self);
 rwlock_endread(&self->f_lock);
 return 0;
err_df:
 rwlock_endread(&self->f_lock);
 err_dead_frame(self);
err:
 return -1;
}

PRIVATE DREF DeeObject *DCALL
frame_getsp_obj(Frame *__restrict self) {
 int32_t result = frame_getsp(self);
 if unlikely(result == -2)
    goto err_df;
 if unlikely(result == -1)
    goto err_unknown;
 return DeeInt_NewU16((uint16_t)result);
err_unknown:
 DeeError_Throwf(&DeeError_ValueError,
                 "Stack depth is unknown");
 goto err;
err_df:
 err_dead_frame(self);
err:
 return NULL;
}

PRIVATE struct type_getset frame_getsets[] = {
    { "location",
     (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&frame_getlocation, NULL, NULL,
      DOC("->?S?T4?X2?Dstring?N?X2?Dint?N?X2?Dint?N?X2?Dstring?N\n"
          "Returns a sequence of tuples describing the frame location, "
          "the first of which is identical to (#file,#line,#col,#name)\n"
          "Rarely ever does the location consist of more than a single "
          "location tuple, however if a function call has been inlined "
          "as a call from another location, the compiler will generate DDI "
          "instrumentation to ensure consistent debug information for both "
          "the inlined function, as well as the call-site") },
    { DeeString_STR(&str_file),
     (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&frame_getfile, NULL, NULL,
      DOC("->?X2?Dstring?N\n"
          "The filename of this frame's source file, or :none when indeterminate") },
    { "line",
     (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&frame_getline, NULL, NULL,
      DOC("->?X2?Dint?N\n"
          "The line number within this frame's source file, or :none when indeterminate") },
    { "col",
     (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&frame_getcol, NULL, NULL,
      DOC("->?X2?Dint?N\n"
          "The column offset within this frame's source file, or :none when indeterminate") },
    { "name",
     (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&frame_getname, NULL, NULL,
      DOC("->?X2?Dstring?N\n"
          "The name of this frame's function, or :none when indeterminate") },
    { "func",
     (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&frame_getfunc, NULL, NULL,
      DOC("->?X2?Dfunction?N\n"
          "Returns the function that is referenced by @this frame, or :none if not available") },
    { "__iswritable__",
     (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&frame_iswritable, NULL, NULL,
      DOC("->?Dbool\n"
          "Evaluates to :true if @this frame is writable") },
    { "__code__",
     (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&frame_getcode, NULL, NULL,
      DOC("->?Ert:Code\n"
          "@throw ReferenceError The frame has continued execution, or was otherwise released\n"
          "The code object that is being executed") },
    { "__pc__",
     (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&frame_getpc, NULL,
     (int(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&frame_setpc,
      DOC("->?Dint\n"
          "@throw ReferenceError The frame has continued execution, or was otherwise released\n"
          "@throw ValueError Attampted to set PC within a read-only frame\n"
          "The current program counter") },
    { "__sp__",
     (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&frame_getsp_obj, NULL, NULL,
      DOC("->?Dint\n"
          "@throw ReferenceError The frame has continued execution, or was otherwise released\n"
          "@throw ValueError The stack depth was undefined and could not be determined\n"
          "Get the current stack depth (same as ${#this.__stack__})\n"
          "To modify this value, use #__stack__ to append/pop objects") },
    /* TODO: __stack__ (read-write, custom, modifiable & resizable, sequence-like object)
     * TODO: __args__ (readonly, custom, sequence-like object for accessing arguments)
     *                 -> reading is the same as it would for the `push arg ...' instruction
     * TODO: __thisarg__ (readonly) access to the this argument
     * TODO: __return__ (read-write) access to the return register
     * TODO: __locals__ (custom, modifiable, sequence-like object)
     * TODO: __variables__ (A special sequence that combines `__locals__' with `__stack__',
     *                      allowing for universal indices to identify objects with local
     *                      life-time, which are then returned by other members, such as `__names__')
     *                      When the stack depth is unknown, simply return `__locals__'
     * TODO: __names__ (Access to a mapping-like object for converting local/stack
     *                  names to indices into the `__variables__' sequence)
     *                  >> local vars = f.__variables__;
     *                  >> for (local name,addr: f.__names__) {
     *                  >>     print "{}@{} = {}".format({
     *                  >>         name,
     *                  >>         addr,
     *                  >>         vars[addr] is bound ? repr vars[addr] : "<unbound>"
     *                  >>     });
     *                  >> }
     * TODO: __symbols__ (custom, modifiable (for some elements), mapping-like object)
     *                 -> By using DDI information for names of locals/stack symbols,
     *                    and combining this information with knowledge of argument
     *                    names, as well as adding special cases for `this' and `return',
     *                    provide access to symbols visible within user-code via their
     *                    individual names.
     *                    Resolution name order here is:
     *                    LOCALS/STACK (DDI) -> ARGUMENTS -> STATIC/CONST -> REF -> `return' + `this'
     */
    { NULL }
};

PRIVATE struct type_member frame_members[] = {
    TYPE_MEMBER_FIELD("__owner__",STRUCT_OBJECT,offsetof(Frame,f_owner)),
    TYPE_MEMBER_END
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
                TYPE_FIXED_ALLOCATOR(Frame)
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
    /* .tp_members       = */frame_members,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */NULL
};

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_FRAME_C */
