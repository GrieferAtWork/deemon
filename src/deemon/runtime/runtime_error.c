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
#ifndef GUARD_DEEMON_RUNTIME_RUNTIME_ERROR_C
#define GUARD_DEEMON_RUNTIME_RUNTIME_ERROR_C 1

#include <deemon/api.h>
#include <deemon/object.h>
#include <deemon/error.h>
#include <deemon/error_types.h>
#include <deemon/tuple.h>
#include <deemon/bytes.h>
#include <deemon/module.h>
#include <deemon/string.h>
#include <deemon/code.h>
#include <deemon/class.h>

#include "runtime_error.h"

#ifndef CONFIG_NO_STDIO
#ifdef CONFIG_HOST_WINDOWS
#include <Windows.h>
#endif
#endif /* !CONFIG_NO_STDIO */

DECL_BEGIN

PUBLIC ATTR_COLD int DCALL
Dee_BadAlloc(size_t req_bytes) {
 DeeNoMemoryErrorObject *nomem_error; int result;
 nomem_error = DeeObject_TRYMALLOC(DeeNoMemoryErrorObject);
 if (!nomem_error) {
  /* If we can't even allocate the no-memory
   * object, throw a static instance. */
  return DeeError_Throw(&DeeError_NoMemory_instance);
 }
 DeeObject_Init(nomem_error,&DeeError_NoMemory);
 nomem_error->e_message    = NULL;
 nomem_error->e_inner      = NULL;
 nomem_error->nm_allocsize = req_bytes;
 /* Throw the no-memory error. */
 result = DeeError_Throw((DeeObject *)nomem_error);
 Dee_Decref(nomem_error);
 return result;
}

INTERN ATTR_COLD int DCALL err_no_active_exception(void) {
 return DeeError_Throwf(&DeeError_RuntimeError,"No active exception");
}

INTERN ATTR_COLD int DCALL
err_subclass_final_type(DeeTypeObject *__restrict tp) {
 ASSERT_OBJECT(tp);
 ASSERT(DeeType_Check(tp));
 return DeeError_Throwf(&DeeError_ValueError,
                        "Cannot create sub-class of final type `%k'",tp);
}

PUBLIC ATTR_COLD int
(DCALL DeeObject_TypeAssertFailed)(DeeObject *__restrict self,
                                   DeeTypeObject *__restrict wanted_type) {
 ASSERT_OBJECT(self);
 ASSERT_OBJECT(wanted_type);
 ASSERT(DeeType_Check(wanted_type));
 return DeeError_Throwf(&DeeError_TypeError,
                        "Expected instance of `%k', but got a `%k' object: %k",
                        wanted_type,Dee_TYPE(self),self);
}

INTERN ATTR_COLD int DCALL
err_unimplemented_constructor(DeeTypeObject *__restrict tp,
                              size_t argc, DeeObject **__restrict argv) {
 DeeObject *args; int result;
 ASSERT_OBJECT(tp);
 ASSERT(DeeType_Check(tp));
 /* XXX: This can be done more efficiently... */
 args = DeeTuple_NewVectorSymbolic(argc,argv);
 if unlikely(!args) return -1;
 result = DeeError_Throwf(&DeeError_NotImplemented,
                          "Constructor `%k%K' is not implemented",
                          tp,DeeTuple_Types(args));
 DeeTuple_DecrefSymbolic(args);
 return result;
}

INTERN ATTR_COLD int DCALL
err_divide_by_zero(DeeObject *__restrict a, DeeObject *__restrict b) {
 ASSERT_OBJECT(a);
 ASSERT_OBJECT(b);
 return DeeError_Throwf(&DeeError_DivideByZero,
                        "Divide by Zero: `%k / %k'",a,b);
}
INTERN ATTR_COLD int DCALL
err_divide_by_zero_i(dssize_t a) {
 return DeeError_Throwf(&DeeError_DivideByZero,
                        "Divide by Zero: `%Id / 0'",a);
}
INTERN ATTR_COLD int DCALL
err_shift_negative(DeeObject *__restrict a, DeeObject *__restrict b, bool is_left_shift) {
 ASSERT_OBJECT(a);
 ASSERT_OBJECT(b);
 return DeeError_Throwf(&DeeError_NegativeShift,
                        "Negative %s shift: `%k %s %k'",
                        is_left_shift ? "left" : "right",a,b,
                        is_left_shift ? "<<" : ">>");
}
INTERN ATTR_COLD int DCALL
err_cannot_weak_reference(DeeObject *__restrict ob) {
 ASSERT_OBJECT(ob);
 return DeeError_Throwf(&DeeError_TypeError,
                        "Cannot create weak reference for instances of type `%k'",
                        Dee_TYPE(ob));
}
INTERN ATTR_COLD int DCALL err_cannot_lock_weakref(void) {
 return DeeError_Throwf(&DeeError_ReferenceError,
                        "Cannot lock weak reference");
}
INTERN ATTR_COLD int DCALL
err_bytes_not_writable(DeeObject *__restrict UNUSED(bytes_ob)) {
 return DeeError_Throwf(&DeeError_BufferError,
                        "The bytes object is not writable");
}

INTERN ATTR_COLD int DCALL
err_unimplemented_operator(DeeTypeObject *__restrict tp, uint16_t operator_name) {
 struct opinfo *info = Dee_OperatorInfo(Dee_TYPE(tp),operator_name);
 ASSERT_OBJECT(tp);
 ASSERT(DeeType_Check(tp));
 return DeeError_Throwf(&DeeError_NotImplemented,
                        "Operator `%k.__%s__' is not implemented",
                        tp,info ? info->oi_sname : "??" "?");
}
INTERN ATTR_COLD int DCALL
err_unimplemented_operator2(DeeTypeObject *__restrict tp,
                            uint16_t operator_name,
                            uint16_t operator_name2) {
 struct opinfo *info,*info2;
 info  = Dee_OperatorInfo(Dee_TYPE(tp),operator_name);
 info2 = Dee_OperatorInfo(Dee_TYPE(tp),operator_name2);
 ASSERT_OBJECT(tp);
 ASSERT(DeeType_Check(tp));
 return DeeError_Throwf(&DeeError_NotImplemented,
                        "Neither `%k.__%s__', nor `%k.__%s__' are implemented",
                        tp,info ? info->oi_sname : "??" "?",
                        tp,info2 ? info2->oi_sname : "??" "?");
}
INTERN ATTR_COLD int DCALL
err_unimplemented_operator3(DeeTypeObject *__restrict tp,
                            uint16_t operator_name,
                            uint16_t operator_name2,
                            uint16_t operator_name3) {
 struct opinfo *info,*info2,*info3;
 info  = Dee_OperatorInfo(Dee_TYPE(tp),operator_name);
 info2 = Dee_OperatorInfo(Dee_TYPE(tp),operator_name2);
 info3 = Dee_OperatorInfo(Dee_TYPE(tp),operator_name3);
 ASSERT_OBJECT(tp);
 ASSERT(DeeType_Check(tp));
 return DeeError_Throwf(&DeeError_NotImplemented,
                        "Neither `%k.__%s__', nor `%k.__%s__', nor `%k.__%s__' are implemented",
                        tp,info ? info->oi_sname : "??" "?",
                        tp,info2 ? info2->oi_sname : "??" "?",
                        tp,info3 ? info3->oi_sname : "??" "?");
}
INTERN ATTR_COLD int DCALL
err_index_out_of_bounds(DeeObject *__restrict self,
                        size_t index, size_t size) {
 ASSERT_OBJECT(self);
 return DeeError_Throwf(&DeeError_IndexError,
                        "Index `%Iu' lies outside the valid bounds `0...%Iu' of sequence of type `%k'",
                        index,size,Dee_TYPE(self));
}
INTERN ATTR_COLD int DCALL
err_index_out_of_bounds_ob(DeeObject *__restrict self,
                           DeeObject *__restrict index) {
 return DeeError_Throwf(&DeeError_IndexError,
                        "Index `%r' lies outside the valid bounds `0...%R' of sequence of type `%k'",
                        index,DeeObject_SizeObject(self),Dee_TYPE(self));
}
INTERN ATTR_COLD int DCALL
err_va_index_out_of_bounds(size_t index, size_t size) {
 ASSERT(index >= size);
 return DeeError_Throwf(&DeeError_IndexError,
                        "Index `%Iu' lies outside the valid bounds `0...%Iu' of varargs",
                        index,size);
}
INTERN ATTR_COLD int DCALL
err_index_unbound(DeeObject *__restrict self, size_t index) {
 ASSERT_OBJECT(self);
 return DeeError_Throwf(&DeeError_UnboundItem,
                        "Index `%Iu' of instance of `%k': %k has not been bound",
                        index,Dee_TYPE(self),self);
}
INTERN ATTR_COLD int DCALL
err_expected_single_character_string(DeeObject *__restrict str) {
 size_t length;
 ASSERT_OBJECT(str);
 ASSERT(DeeString_Check(str) || DeeBytes_Check(str));
 length = DeeString_Check(str) ? DeeString_WLEN(str) : DeeBytes_SIZE(str);
 return DeeError_Throwf(&DeeError_ValueError,
                        "Expected a single character but got %Iu characters in %r",
                        length,str);
}
INTERN ATTR_COLD int DCALL
err_expected_string_for_attribute(DeeObject *__restrict but_instead_got) {
 ASSERT_OBJECT(but_instead_got);
 ASSERT(DeeString_Check(but_instead_got));
 return DeeError_Throwf(&DeeError_TypeError,
                        "Expected string for attribute, but got instance of `%k': %k",
                        Dee_TYPE(but_instead_got),but_instead_got);
}
INTERN ATTR_COLD int DCALL
err_integer_overflow(DeeObject *__restrict overflowing_object,
                     size_t cutoff_bits, bool positive_overflow) {
 ASSERT_OBJECT(overflowing_object);
 if (!cutoff_bits) {
  return DeeError_Throwf(&DeeError_IntegerOverflow,
                         "%s integer overflow in %k",
                         positive_overflow ? "positive" : "negative",
                         overflowing_object);
 }
 return DeeError_Throwf(&DeeError_IntegerOverflow,
                        "%s integer overflow after %Iu bits in %k",
                        positive_overflow ? "positive" : "negative",
                        cutoff_bits,overflowing_object);
}
INTERN ATTR_COLD int DCALL
err_integer_overflow_i(size_t cutoff_bits, bool positive_overflow) {
 if (!cutoff_bits) {
  return DeeError_Throwf(&DeeError_IntegerOverflow,
                         "%s integer overflow",
                         positive_overflow ? "positive" : "negative");
 }
 return DeeError_Throwf(&DeeError_IntegerOverflow,
                        "%s integer overflow after %Iu bits",
                        positive_overflow ? "positive" : "negative",
                        cutoff_bits);
}
INTERN ATTR_COLD int DCALL
err_keywords_not_accepted(DeeTypeObject *__restrict tp_self,
                          DeeObject *__restrict kw) {
 return DeeError_Throwf(&DeeError_TypeError,
                        "Instance of %k does not accept keyword arguments %r",
                        tp_self,kw);
}
INTERN ATTR_COLD int DCALL
err_keywords_func_not_accepted(char const *__restrict name,
                               DeeObject *__restrict kw) {
 return DeeError_Throwf(&DeeError_TypeError,
                        "Function %s does not accept keyword arguments %r",
                        name,kw);
}
INTERN ATTR_COLD int DCALL
err_keywords_ctor_not_accepted(DeeTypeObject *__restrict tp_self,
                               DeeObject *__restrict kw) {
 return DeeError_Throwf(&DeeError_TypeError,
                        "Constructor for %k does not accept keyword arguments %r",
                        tp_self,kw);
}
INTERN ATTR_COLD int DCALL
err_keywords_bad_for_argc(size_t argc, size_t kwdc) {
 return DeeError_Throwf(&DeeError_TypeError,
                        "Invalid keyword list containing %Iu keywords "
                        "when only %Iu arguments were given",
                        kwdc,argc);
}
INTERN ATTR_COLD int DCALL
err_keywords_not_found(char const *__restrict keyword) {
 return DeeError_Throwf(&DeeError_TypeError,
                        "Missing argument %s",
                        keyword);
}
INTERN ATTR_COLD int DCALL
err_invalid_segment_size(size_t segsz) {
 return DeeError_Throwf(&DeeError_ValueError,
                        "Invalid segment size: %Iu",
                        segsz);
}
INTERN ATTR_COLD int DCALL
err_invalid_distribution_count(size_t distcnt) {
 return DeeError_Throwf(&DeeError_ValueError,
                        "Invalid distribution count: %Iu",
                        distcnt);
}
INTERN ATTR_COLD int DCALL
err_invalid_argc(char const *function_name, size_t argc_cur,
                 size_t argc_min, size_t argc_max) {
 if (argc_min == argc_max) {
  return DeeError_Throwf(&DeeError_TypeError,
                         "function%s%s expects %Iu arguments when %Iu w%s given",
                         function_name ? " " : "",function_name ? function_name : "",
                         argc_min,argc_cur,argc_cur == 1 ? "as" : "ere");
 } else {
  return DeeError_Throwf(&DeeError_TypeError,
                         "function%s%s expects between %Iu and %Iu arguments when %Iu w%s given",
                         function_name ? " " : "",function_name ? function_name : "",
                         argc_min,argc_max,argc_cur,argc_cur == 1 ? "as" : "ere");
 }
}
INTERN ATTR_COLD int DCALL
err_invalid_argc_va(char const *function_name, size_t argc_cur, size_t argc_min) {
 return DeeError_Throwf(&DeeError_TypeError,
                        "function%s%s expects at least %Iu arguments when only %Iu w%s given",
                        function_name ? " " : "",function_name ? function_name : "",
                        argc_min,argc_cur,argc_cur == 1 ? "as" : "ere");
}
INTERN ATTR_COLD int DCALL
err_invalid_argc_unpack(DeeObject *__restrict unpack_object,
                        size_t argc_cur, size_t argc_min, size_t argc_max) {
 ASSERT_OBJECT(unpack_object);
 (void)unpack_object;
 if (argc_min == argc_max) {
  return DeeError_Throwf(&DeeError_UnpackError,
                         "Expected %Iu object%s when %Iu w%s given",
                         argc_min,argc_min > 1 ? "s" : "",argc_cur,
                         argc_min == 1 ? "as" : "ere");
 } else {
  return DeeError_Throwf(&DeeError_UnpackError,
                         "Expected between %Iu and %Iu objects when %Iu were given",
                         argc_min,argc_max,argc_cur);
 }
}
INTERN ATTR_COLD int DCALL
err_invalid_unpack_size(DeeObject *__restrict unpack_object,
                        size_t need_size, size_t real_size) {
 ASSERT_OBJECT(unpack_object);
 return DeeError_Throwf(&DeeError_UnpackError,
                        "Expected %Iu object%s when %Iu w%s given",
                        need_size,need_size > 1 ? "s" : "",real_size,
                        real_size == 1 ? "as" : "ere");
}
INTERN ATTR_COLD int DCALL
err_invalid_va_unpack_size(size_t need_size, size_t real_size) {
 return DeeError_Throwf(&DeeError_UnpackError,
                        "Expected %Iu object%s when %Iu w%s given",
                        need_size,need_size > 1 ? "s" : "",real_size,
                        real_size == 1 ? "as" : "ere");
}
INTERN ATTR_COLD int DCALL
err_invalid_unpack_iter_size(DeeObject *__restrict unpack_object,
                             DeeObject *__restrict unpack_iterator,
                             size_t need_size) {
 ASSERT_OBJECT(unpack_object);
 ASSERT_OBJECT(unpack_iterator);
 (void)unpack_object;
 (void)unpack_iterator;
 return DeeError_Throwf(&DeeError_UnpackError,
                        "Expected %Iu object%s when at least %Iu w%s given",
                        need_size,need_size > 1 ? "s" : "",need_size+1,
                        need_size == 0 ? "as" : "ere");
}
INTERN ATTR_COLD int DCALL
err_unbound_global(DeeModuleObject *__restrict module,
                   uint16_t global_index) {
 DeeObject *name;
 ASSERT_OBJECT(module);
 ASSERT(DeeModule_Check(module));
 ASSERT(global_index < module->mo_globalc);
 name = DeeModule_GlobalName((DeeObject *)module,global_index);
 return DeeError_Throwf(&DeeError_UnboundLocal, /* XXX: UnboundGlobal? */
                        "Unbound global variable `%s' from `%s'",
                        name ? DeeString_STR(name) : "??" "?",
                        DeeString_STR(module->mo_name));
}
INTERN ATTR_COLD int DCALL
err_unbound_local(struct code_object *__restrict code,
                  void *__restrict ip, uint16_t local_index) {
 char const *local_name;
 char const *code_name;
 ASSERT_OBJECT(code);
 ASSERT(DeeCode_Check(code));
 ASSERT(local_index < code->co_localc);
 code_name = DeeDDI_NAME(code->co_ddi);
 if (!*code_name) code_name = NULL;
#if 1
 local_name = NULL;
 (void)ip;
#else /* TODO: Use DDI information to lookup the name of the variable. */
 if (local_index >= code->co_ddi->d_nlocals) local_name = NULL;
 else if (*(local_name = DeeDDI_LOCAL_NAME(code->co_ddi,local_index)) == '\0') local_name = NULL;
#endif
 if (local_name) {
  return DeeError_Throwf(&DeeError_UnboundLocal,
                         "Unbound local variable `%s' %s%s",local_name,
                         code_name ? " in function " : "",
                         code_name ? code_name : "");
 } else {
  return DeeError_Throwf(&DeeError_UnboundLocal,
                         "Unbound local variable %d%s%s",local_index,
                         code_name ? " in function " : "",
                         code_name ? code_name : "");
 }
}
INTERN ATTR_COLD int DCALL
err_illegal_instruction(struct code_object *__restrict code,
                        void *__restrict ip) {
 char const *code_name = DeeDDI_NAME(code->co_ddi);
 if (!*code_name) code_name = "??" "?";
 return DeeError_Throwf(&DeeError_IllegalInstruction,
                        "Illegal instruction at %s+%.4I32X",
                        code_name,(uint32_t)((instruction_t *)ip - code->co_code));
}
INTERN ATTR_COLD int DCALL
err_requires_class(DeeTypeObject *__restrict tp_self) {
 return DeeError_Throwf(&DeeError_TypeError,
                        "Needed a class when %k is only a regular type",
                        tp_self);
}
INTERN ATTR_COLD int DCALL
err_invalid_class_index(DeeTypeObject *__restrict tp_self,
                        DeeObject *__restrict UNUSED(self),
                        unsigned int index) {
 return DeeError_Throwf(&DeeError_TypeError,
                        "Invalid class index %u for %k",
                        index,tp_self);
}


INTERN ATTR_COLD int DCALL
err_invalid_refs_size(DeeObject *__restrict code, size_t num_refs) {
 ASSERT_OBJECT_TYPE_EXACT(code,&DeeCode_Type);
 return DeeError_Throwf(&DeeError_TypeError,
                        "Code object expects %I16u references when %Iu were given",
                       ((DeeCodeObject *)code)->co_refc,num_refs);
}



PRIVATE char const access_names[4][4] = {
   /* [ATTR_ACCESS_GET] = */"get",
   /* [ATTR_ACCESS_DEL] = */"del",
   /* [ATTR_ACCESS_SET] = */"set",
   /* [?]               = */"",
};
INTERN ATTR_COLD int DCALL
err_unknown_attribute(DeeTypeObject *__restrict tp,
                      char const *__restrict name,
                      int access) {
 ASSERT_OBJECT(tp);
 ASSERT(name);
 ASSERT(DeeType_Check(tp));
 return DeeError_Throwf(&DeeError_AttributeError,
                        "Cannot %s unknown attribute `%k.%s'",
                        access_names[access&ATTR_ACCESS_MASK],tp,name);
}
INTERN ATTR_COLD int DCALL
err_nodoc_attribute(char const *base,
                    char const *__restrict name) {
 return DeeError_Throwf(&DeeError_ValueError,
                        "No documentation found for `%s.%s'",
                        base ? base : "?",name);
}
INTERN ATTR_COLD int DCALL
err_cant_access_attribute(DeeTypeObject *__restrict tp,
                          char const *__restrict name,
                          int access) {
 ASSERT_OBJECT(tp);
 ASSERT(name);
 ASSERT(DeeType_Check(tp));
 return DeeError_Throwf(&DeeError_AttributeError,
                        "Cannot %s attribute `%k.%s'",
                        access_names[access&ATTR_ACCESS_MASK],
                        tp,name);
}
INTERN ATTR_COLD int DCALL
err_unbound_attribute(DeeTypeObject *__restrict tp, char const *__restrict name) {
 ASSERT_OBJECT(tp);
 ASSERT(name);
 ASSERT(DeeType_Check(tp));
 return DeeError_Throwf(&DeeError_UnboundAttribute,
                        "Unbound attribute `%k.%s'",
                        tp,name);
}
INTERN ATTR_COLD int DCALL
err_unknown_key(DeeObject *__restrict map, DeeObject *__restrict key) {
 ASSERT_OBJECT(map);
 ASSERT_OBJECT(key);
 return DeeError_Throwf(&DeeError_KeyError,
                        "Could not find key `%k' in %k `%k'",
                        key,Dee_TYPE(map),map);
}
INTERN ATTR_COLD int DCALL
err_unknown_key_str(DeeObject *__restrict map, char const *__restrict key) {
 ASSERT_OBJECT(map);
 ASSERT(key);
 return DeeError_Throwf(&DeeError_KeyError,
                        "Could not find key `%s' in %k `%k'",
                        key,Dee_TYPE(map),map);
}
INTERN ATTR_COLD int DCALL
err_empty_sequence(DeeObject *__restrict seq) {
 ASSERT_OBJECT(seq);
 return DeeError_Throwf(&DeeError_ValueError,
                        "Empty sequence of type `%k' encountered",
                        Dee_TYPE(seq));
}
INTERN ATTR_COLD int DCALL
err_changed_sequence(DeeObject *__restrict seq) {
 ASSERT_OBJECT(seq);
 return DeeError_Throwf(&DeeError_RuntimeError,
                        "A sequence `%k' has changed while being iterated: `%k'",
                        Dee_TYPE(seq),seq);
}

INTERN ATTR_COLD int DCALL
err_immutable_sequence(DeeObject *__restrict self) {
 return DeeError_Throwf(&DeeError_SequenceError,
                        "Instances of sequence type `%k' are immutable",
                        Dee_TYPE(self));
}
INTERN ATTR_COLD int DCALL
err_fixedlength_sequence(DeeObject *__restrict self) {
 return DeeError_Throwf(&DeeError_SequenceError,
                        "Instances of sequence type `%k' have a fixed length",
                        Dee_TYPE(self));
}

INTERN ATTR_COLD int DCALL
err_item_not_found(DeeObject *__restrict seq, DeeObject *__restrict item) {
 ASSERT_OBJECT(seq);
 return DeeError_Throwf(&DeeError_ValueError,
                        "Could not locate item `%k' in sequence `%k'",
                        item,seq);
}
INTERN ATTR_COLD int DCALL
err_index_not_found(DeeObject *__restrict seq, DeeObject *__restrict item) {
 ASSERT_OBJECT(seq);
 return DeeError_Throwf(&DeeError_IndexError,
                        "Could not locate item `%k' in sequence `%k'",
                        item,seq);
}
INTERN ATTR_COLD int DCALL
err_protected_member(DeeTypeObject *__restrict class_type,
                     struct member_entry *__restrict member) {
 ASSERT_OBJECT_TYPE(class_type,&DeeType_Type);
 ASSERT(DeeType_IsClass(class_type));
 ASSERT(member);
 return DeeError_Throwf(&DeeError_AttributeError,
                        "Cannot access %s member `%k' of class `%k'",
                       (member->ca_flag&CLASS_MEMBER_FPRIVATE) ? "private" : "public",
                        member->cme_name,class_type);
}
INTERN ATTR_COLD int DCALL
err_no_super_class(DeeTypeObject *__restrict type) {
 ASSERT_OBJECT_TYPE(type,&DeeType_Type);
 return DeeError_Throwf(&DeeError_TypeError,
                        "Type `%k' has no super-class",type);
}

INTERN ATTR_COLD int DCALL
err_file_not_found(char const *__restrict filename) {
 return DeeError_Throwf(&DeeError_FileNotFound,
                        "File `%s' could not be found",filename);
}

#ifndef CONFIG_NO_STDIO
#ifdef CONFIG_HOST_WINDOWS
INTERN ATTR_NOINLINE ATTR_COLD int DCALL
err_system_error_code(char const *__restrict function,
                      unsigned long last_error) {
 DeeError_Throwf(&DeeError_SystemError,
                 "System function `%s' failed: %lx",
                 function,last_error);
 return -1;
}
INTERN ATTR_NOINLINE ATTR_COLD int DCALL
err_system_error(char const *__restrict function) {
 return err_system_error_code(function,GetLastError());
}
#endif
#endif /* !CONFIG_NO_STDIO */



INTERN ATTR_COLD int DCALL
err_srt_invalid_sp(struct code_frame *__restrict frame, size_t access_sp) {
 return DeeError_Throwf(&DeeError_SegFault,
                        "Unbound stack variable %Iu lies above active end %Iu",
                        access_sp,frame->cf_stacksz);
}
PRIVATE ATTR_COLD int DCALL
err_srt_invalid_symid(struct code_frame *__restrict UNUSED(frame),
                      char category, uint16_t id) {
 return DeeError_Throwf(&DeeError_IllegalInstruction,
                        "Attempted to access invalid %cID %I16u",
                        category,id);
}
INTERN ATTR_COLD int DCALL
err_srt_invalid_static(struct code_frame *__restrict frame, uint16_t sid) {
 return err_srt_invalid_symid(frame,'S',sid);
}
INTERN ATTR_COLD int DCALL
err_srt_invalid_const(struct code_frame *__restrict frame, uint16_t cid) {
 return err_srt_invalid_symid(frame,'C',cid);
}
INTERN ATTR_COLD int DCALL
err_srt_invalid_locale(struct code_frame *__restrict frame, uint16_t lid) {
 return err_srt_invalid_symid(frame,'L',lid);
}
INTERN ATTR_COLD int DCALL
err_srt_invalid_ref(struct code_frame *__restrict frame, uint16_t rid) {
 return err_srt_invalid_symid(frame,'R',rid);
}
INTERN ATTR_COLD int DCALL
err_srt_invalid_module(struct code_frame *__restrict frame, uint16_t mid) {
 return err_srt_invalid_symid(frame,'M',mid);
}
INTERN ATTR_COLD int DCALL
err_srt_invalid_global(struct code_frame *__restrict frame, uint16_t gid) {
 return err_srt_invalid_symid(frame,'G',gid);
}
INTERN ATTR_COLD int DCALL
err_srt_invalid_extern(struct code_frame *__restrict frame, uint16_t mid, uint16_t gid) {
 if (mid >= frame->cf_func->fo_code->co_module->mo_importc)
     return err_srt_invalid_module(frame,mid);
 return err_srt_invalid_global(frame,gid);
}


DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_RUNTIME_ERROR_C */
